#!/bin/bash
# NetworkManager dispatcher: auto-open Project Civic dashboard
# when STM32 RNDIS interface connects via USB OTG
#
# Install:
#   sudo cp scripts/99-civic-dashboard.sh /etc/NetworkManager/dispatcher.d/99-civic-dashboard
#   sudo chmod 755 /etc/NetworkManager/dispatcher.d/99-civic-dashboard

IFACE="$1"
ACTION="$2"

[ "$ACTION" != "up" ] && exit 0

MAC=$(cat "/sys/class/net/$IFACE/address" 2>/dev/null)
case "$MAC" in
    02:43:49:56:49:*) ;;
    *) exit 0 ;;
esac

sleep 1

# Find active graphical session user via loginctl (avoids AppArmor-blocked 'who')
DISPLAY_USER=$(loginctl list-sessions --no-legend 2>/dev/null | awk '$4 != "" {print $3; exit}')
[ -z "$DISPLAY_USER" ] && DISPLAY_USER="${SUDO_USER:-$(logname 2>/dev/null)}"
[ -z "$DISPLAY_USER" ] && exit 0

# Open dashboard (supports both Wayland and X11)
USER_UID=$(id -u "$DISPLAY_USER")
USER_RUN="/run/user/$USER_UID"

# Try Wayland first (DBUS_SESSION_BUS_ADDRESS needed for xdg-open under Wayland)
if [ -d "$USER_RUN" ]; then
    sudo -u "$DISPLAY_USER" \
        DBUS_SESSION_BUS_ADDRESS="unix:path=$USER_RUN/bus" \
        XDG_RUNTIME_DIR="$USER_RUN" \
        xdg-open "http://192.168.7.1/" &>/dev/null &
else
    # Fallback X11
    XAUTH="/home/$DISPLAY_USER/.Xauthority"
    sudo -u "$DISPLAY_USER" DISPLAY=:0 XAUTHORITY="$XAUTH" \
        xdg-open "http://192.168.7.1/" &>/dev/null &
fi

exit 0
