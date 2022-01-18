#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dbus-1.0/dbus/dbus.h>

/* Battery notifier settigns */
typedef struct {
    unsigned charge;
    const char* alert;
    const char* icon;
} Rule;

static char* fileCapacity = "/sys/class/power_supply/BAT0/capacity";
static char* fileAc       = "/sys/class/power_supply/AC/online";
static int interval = 30; // as seconds
static Rule chargeLevels[] = {
    {5,   "critically low!", "battery_alert"}, // Charge, notification, icon
    {15,  "low",             "battery_20"},
    {30,  "notice",          "battery_30"},

    {90,  "full",            "battery_full"}, // Full at
    {200, "charging",        "battery_charging_full"} // Charging
};

typedef enum { 
    CHARGING,
    DISCHARGING
} ChargingStatus;

typedef struct {
    unsigned charge;
    ChargingStatus status;
    bool full;
} BatteryStatus;

/* Notification settings */
enum { LOW, NORMAL, CRITICAL };
static unsigned char level = LOW;
static char* application = "battonotif";
static char* urgency = "urgency";
static int timeout = 1000 * 3; // as milliseconds
static unsigned id = 2593;

/* Globals */
BatteryStatus battery;
char ntitle[25], nbody[40];



void notify(const char* summary, const char* body, const char* icon) {
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, 0);
    DBusMessage* message = dbus_message_new_method_call("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
    DBusMessageIter iter[4];
    dbus_message_iter_init_append(message, iter);
    
    dbus_message_iter_append_basic(iter, 's', &application);
    dbus_message_iter_append_basic(iter, 'u', &id);
    dbus_message_iter_append_basic(iter, 's', &icon);
    dbus_message_iter_append_basic(iter, 's', &summary);
    
    dbus_message_iter_append_basic(iter, 's', &body);
    dbus_message_iter_open_container(iter, 'a', "s", iter + 1);
    dbus_message_iter_close_container(iter, iter + 1);
    dbus_message_iter_open_container(iter, 'a', "{sv}", iter + 1);
    dbus_message_iter_open_container(iter + 1, 'e', 0, iter + 2);
    
    dbus_message_iter_append_basic(iter + 2, 's', &urgency);
    dbus_message_iter_open_container(iter + 2, 'v', "y", iter + 3);
    dbus_message_iter_append_basic(iter + 3, 'y', &level);
    dbus_message_iter_close_container(iter + 2, iter + 3);
    dbus_message_iter_close_container(iter + 1, iter + 2);
    dbus_message_iter_close_container(iter, iter + 1);
    
    dbus_message_iter_append_basic(iter, 'i', &timeout);
    dbus_connection_send(connection, message, 0);
    dbus_connection_flush(connection);
    dbus_message_unref(message);
    dbus_connection_unref(connection);
}

void fail(const char* msg) {
    printf(msg);
    printf("\n\nUsage: battnotif\n");
    exit(1);
}

void getBatteryStatus() {
    FILE* file;
    int charging;

    file = fopen(fileCapacity, "r");
    if (file == NULL)
        fail("Couldn't open battery capacity file. Exiting...");
    fscanf(file, "%d", &(battery.charge));
    fclose(file);
    
    file = fopen(fileAc, "r");
    if (file == NULL)
        fail("Couldn't open battery status file. Exiting...");
    
    charging = fgetc(file);
    fclose(file);
    
    battery.full = (battery.charge >= chargeLevels[sizeof(chargeLevels) / sizeof(Rule) - 2].charge);

    if ((char)charging == '1') {
        battery.status = CHARGING;
    } else {
        battery.status = DISCHARGING;
    }
}

void checkRules() {
    getBatteryStatus();
    BatteryStatus lastStatusAlert = { 100, battery.status, false };
    
    for (;;) {
        sleep(interval);
        getBatteryStatus();
        
        if (battery.status == DISCHARGING) {
            if (lastStatusAlert.status != DISCHARGING) { // Check if status wasnt discharging before
                lastStatusAlert.status = DISCHARGING;
                lastStatusAlert.full = false;

                sprintf(nbody, "Battery charge at %d%%", battery.charge);
                notify("Battery discharging", nbody, chargeLevels[0].icon);
            } else { // If status was discharging before
                for (unsigned i = 0; i < (unsigned)(sizeof(chargeLevels) / sizeof(Rule)) - 2; i++) { // Loop through all rules, except last one as it's reserved for full battery check
                    if (battery.charge <= chargeLevels[i].charge) { // Check if charge is less that set rule
                        if (lastStatusAlert.charge != chargeLevels[i].charge) { // Check if alert hasn't already been notified
                            lastStatusAlert.charge = chargeLevels[i].charge;

                            sprintf(ntitle, "Battery %s", chargeLevels[i].alert);
                            sprintf(nbody, "Battery charge at %d%%", battery.charge);
                            notify(ntitle, nbody, chargeLevels[i].icon);
                        }
                        break;
                    }
                }
            }
        } else { // If battery is charging
            if (!lastStatusAlert.full & battery.full) {
                lastStatusAlert.full = true;
                notify("Battery full", "Battery charge full", chargeLevels[sizeof(chargeLevels) / sizeof(Rule) - 2].icon);
            } else if (lastStatusAlert.status != CHARGING) {
                lastStatusAlert.status = CHARGING;
                sprintf(nbody, "Battery charging at %d%%", battery.charge);
                notify("Battery charging", nbody, chargeLevels[sizeof(chargeLevels) / sizeof(Rule) - 1].icon);
            }

        }
    }
}



int main(void) {
    checkRules();
    return 0;
}
