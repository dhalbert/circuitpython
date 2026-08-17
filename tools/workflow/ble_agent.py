#! /usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Dan Halbert for Adafruit Industries
#
# SPDX-License-Identifier: MIT

"""A Just Works BLE pairing agent, so a browser can pair with the BLE workflow.

Linux only. Leave it running in a terminal while you connect.

BlueZ has no pairing UI of its own. Every pairing interaction is delegated over
D-Bus to an org.bluez.Agent1 that some application must register, and whichever
one most recently called RequestDefaultAgent receives the requests.

Chromium is no help, in two different ways. Usually it has no agent registered at
all: it registers one only when bluetoothd starts, and unregisters it on page
load, so by the time you connect there is nothing there and bluetoothd refuses
the pairing outright:

    src/device.c:new_auth() No agent available for request type 2
    device_confirm_passkey: Operation not permitted

When Chromium does hold an agent -- right after a bluetoothd restart, say -- it is
worse, because RequestDefaultAgent puts it first in line and it then answers every
RequestAuthorization with org.bluez.Error.Rejected, no prompt shown. That can
preempt a desktop agent that would have asked. It is why this script insists on
becoming the default agent, and warns when it cannot.

Either way the link drops. The characteristics needing an encrypted link are the File
Transfer transfer characteristic and the BLE serial ones (SECURITY_MODE_ENC_NO_MITM);
the version characteristics are SECURITY_MODE_OPEN, which is why service discovery
and a version read succeed first and the drop looks unprovoked. macOS and Windows do
not need this -- there the operating system owns pairing and shows its own dialog.

This registers a NoInputNoOutput agent, which tells BlueZ not to ask anything, so
pairing completes as Just Works: encrypted, but with no MITM protection, since
neither end can display or confirm a passkey. That is the same security the
workflow already assumes; running this does not weaken it. It is the same agent
ble_terminal.py registers for itself, split out so it can be used with a browser.

    python3 ble_agent.py

Press Ctrl-C to unregister and exit. Requires the dbus-fast package.

Note that an agent only fixes pairing. It does not help with the separate Linux
kernel problem where a connection attempt to an unbonded peripheral often times
out; for that, just retry. Once the board is bonded, both problems go away.
"""

import argparse
import asyncio
import contextlib
import sys

from dbus_fast import BusType, DBusError, Message
from dbus_fast.aio import MessageBus
from dbus_fast.service import ServiceInterface, method

AGENT_PATH = "/org/adafruit/circuitpython_ble_agent"
BLUEZ = "org.bluez"


def status(message):
    print(f"[agent] {message}", file=sys.stderr, flush=True)


class JustWorksAgent(ServiceInterface):
    """NoInputNoOutput pairing agent.

    With NoInputNoOutput capability BlueZ selects Just Works and does not call
    RequestConfirmation or RequestPasskey at all. The methods are still
    implemented because BlueZ requires the full org.bluez.Agent1 interface to be
    present, and because another agent's capability could put us on a path that
    does call them.
    """

    def __init__(self, verbose=False):
        super().__init__("org.bluez.Agent1")
        self._verbose = verbose

    def _trace(self, message):
        if self._verbose:
            status(message)

    @method()
    def Release(self):  # noqa: N802 - D-Bus method names
        self._trace("released by bluetoothd")

    @method()
    def RequestPinCode(self, device: "o") -> "s":  # noqa: N802, F821
        raise DBusError("org.bluez.Error.Rejected", "no input capability")

    @method()
    def RequestPasskey(self, device: "o") -> "u":  # noqa: N802, F821
        raise DBusError("org.bluez.Error.Rejected", "no input capability")

    @method()
    def DisplayPinCode(self, device: "o", pincode: "s"):  # noqa: N802, F821
        self._trace(f"display pin code {pincode} for {device}")

    @method()
    def DisplayPasskey(self, device: "o", passkey: "u", entered: "q"):  # noqa: N802, F821
        self._trace(f"display passkey {passkey:06d} for {device}")

    @method()
    def RequestConfirmation(self, device: "o", passkey: "u"):  # noqa: N802, F821
        # Not reached with NoInputNoOutput. Accept if it ever is: refusing here
        # is exactly the failure this script exists to prevent.
        status(f"confirming pairing with {device} (passkey {passkey:06d})")

    @method()
    def RequestAuthorization(self, device: "o"):  # noqa: N802, F821
        status(f"authorizing pairing with {device}")

    @method()
    def AuthorizeService(self, device: "o", uuid: "s"):  # noqa: N802, F821
        self._trace(f"authorizing service {uuid} for {device}")

    @method()
    def Cancel(self):  # noqa: N802
        status("pairing cancelled by the remote device")


async def call_agent_manager(bus, member, body, signature):
    reply = await bus.call(
        Message(
            destination=BLUEZ,
            path="/org/bluez",
            interface="org.bluez.AgentManager1",
            member=member,
            signature=signature,
            body=body,
        )
    )
    if reply.message_type.name == "ERROR":
        raise RuntimeError(f"{member} failed: {reply.error_name}: {reply.body}")


async def run(verbose):
    try:
        bus = await MessageBus(bus_type=BusType.SYSTEM).connect()
    except Exception as e:
        status(f"could not connect to the system bus: {e}")
        return 1

    bus.export(AGENT_PATH, JustWorksAgent(verbose))

    try:
        await call_agent_manager(bus, "RegisterAgent", [AGENT_PATH, "NoInputNoOutput"], "os")
    except RuntimeError as e:
        status(str(e))
        # RegisterAgent keys on the D-Bus sender, so another application's agent is not
        # the cause; only a second registration on this same connection collides.
        status("is an agent already registered on this connection?")
        return 1

    try:
        await call_agent_manager(bus, "RequestDefaultAgent", [AGENT_PATH], "o")
    except RuntimeError as e:
        # Registered but not default: another agent will get the requests first.
        status(str(e))
        status("registered, but could not become the default agent; pairing may still fail.")
    else:
        status(f"registered as the default agent ({AGENT_PATH}, NoInputNoOutput)")

    status("leave this running while you pair, then press Ctrl-C")
    try:
        await asyncio.get_running_loop().create_future()  # run until cancelled
    finally:
        with contextlib.suppress(Exception):
            await call_agent_manager(bus, "UnregisterAgent", [AGENT_PATH], "o")
            status("unregistered")
        bus.disconnect()
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="log every agent callback, not just the ones that matter",
    )
    args = parser.parse_args()
    try:
        return asyncio.run(run(args.verbose))
    except KeyboardInterrupt:
        return 0


sys.exit(main())
