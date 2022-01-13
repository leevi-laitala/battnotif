#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dbus-1.0/dbus/dbus.h>

/* Battery notifier settigns */
typedef enum { 
    CHARGING,
    DISCHARGING,
    FULLCHARGE
} ChargingStatus;

typedef enum {
    CFULL,
    CNOTICE,
    CLOW,
    CCRIT
} ChargeLevel;

typedef struct {
    int charge;
    ChargingStatus status;
} BatteryStatus;

typedef struct {
    unsigned charge;
    const char* alert;
    const char* icon;
} Rule;

static char* fileCapacity = "/sys/class/power_supply/BAT0/capacity";
static char* fileAc       = "/sys/class/power_supply/AC/online"
static int interval = 10; // as seconds
static Rule chargeLevels[] = {
    {90, "full",            "battery_full"}, // Charge, notification, icon
    {30, "notice",          "battery_30"},
    {15, "low",             "battery_20"},
    {5,  "critically low!", "battery_alert"}
};

/* Notification settings */
enum { LOW, NORMAL, CRITICAL };
static unsigned char level = LOW;
static char* application = "battonotif";
static char* urgency = "urgency";
static char* icons[] = { "volume_mute", "volume_down", "volume_up", "volume_off" }; // Last icon is used when muted
static int timeout = 1000 * 3; // as milliseconds
static unsigned id = 2593;

/* Globals */
BatteryStatus battery;



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
    
    if ((char)charging == '1')
        if (battery.charge > chargeLevels[CFULL].charge)
            battery.status = FULLCHARGE;
        else
            battery.status = CHARGING;
    else
        battery.status = DISCHARGING;
}

int main(int argc, const char* argv[]) {
    unsigned lastChargeLevel = 0;
    
    for (;;) {
        printf("Tick\n");
        //printf("Percent: %d\n", getBatteryPercentage());
        getBatteryStatus();

        if (battery.charge > )

        sleep(interval);
    }

    return 0;
}
