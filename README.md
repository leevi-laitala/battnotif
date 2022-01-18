# Battnotif

Simple battery notifier written in C.

![](resources/preview.gif)

<br>

### Configuration

There are currenlty no command line arguments to be given for the application,
all configuration needs to be done in the source code before compiling.

Below are shown default configurations and their descriptions.

<br>

###### Battery files
```
static char* fileCapacity = "/sys/class/power_supply/BAT0/capacity";
static char* fileAc       = "/sys/class/power_supply/AC/online";
```
Battery and AC adapter files. You can find them from `/sys/class/power_supply/` directory.
The application can't currently search for them automatically, so you have to provide them
manually.

<br>

###### Checking interval
```
static int interval = 30; // as seconds
```
How often the battery and the ac adapter files are checked.


<br>

###### Rules
```
static Rule chargeLevels[] = {
    {5,   "critically low!", "battery_alert"}, // Charge, notification, icon
    {15,  "low",             "battery_20"},
    {30,  "notice",          "battery_30"},

    {90,  "full",            "battery_full"}, // Full at
    {200, "charging",        "battery_charging_full"} // Charging
};
```
Here you set the rules for the notifications. 

Last two entries have special use, but everything before that are treated as alerts. Alerts need to be sorted by
the first variable in ascending order. There is no hardcoded cap, so you can add more if you want.

Second to last entry is where you set when the battery capacity is considered full. And last entry is used when
the AC adapter is online.

<br>

First variable is unsigned integer, which is the charge level
at when notification is sent.

Second variable is string of characters, which is appended in front of string 'Battery', so for
example with default configuration battery at 30% would sent notification with body of 'Battery notice'.

Third variable holds the name of the icon used for the notification at given charge level. 

### Building

```
git clone https://github.com/leevi-laitala/battnotif.git
cd battnotif
make
```

This will build portable application `battnotif`

### Dependencies

dbus-glib


