

static const char * VePrettyData[][3] PROGMEM {
// get name, pretify name, value operator
{"V","Voltage", "1000",}, // display in webUI | Dont edit
{"V2","Voltage_2", "1000",},
{"V3","Voltage_3", "1000",},
{"VS","Starter_voltage", "1000",},
{"VM","Mid_voltage", "1000",},
{"DM","Mid_deviation", "10",},
{"VPV","Panel_voltage", "1000",}, // display in webUI | Dont edit
{"PPV","Panel_power", "0",}, // display in webUI | Dont edit
{"I","Battery_current","1000"}, // display in webUI | Dont edit
{"I2","Battery_current_2","1000"},
{"I3","Battery_current_3","1000"},
{"IL","Load_current","1000"}, // display in webUI | Dont edit
{"LOAD","Load_output_state",""}, // display in webUI | Dont edit
//{"T","Battery_temperature","0"}, // display in webUI | Dont edit
{"P","Instantaneous_power","0"},
{"CE","Consumed_Amp_Hours","1000"},
//{"SOC", "SOC", "1000"}, // display in webUI | Dont edit
{"TTG","Time_to_go","0"},
{"ALARM","Alarm",""},
//{"RELAY","Relay",""}, // display in webUI | Dont edit
{"AR", "Alarm_code", "0"},
//{"OR", "Off_reason", "0"}, // LASTAUSGANG CODES , SIEHE UNTEN 
{"H1", "Deepest_discharge", "1000"},
{"H2", "Last_discharge", "1000"},
{"H3", "Average_discharge", "1000"},
{"H4", "Charge_cycles", "0"},
{"H5", "Full_discharges", "0"},
{"H6", "Cumulative_Ah_drawn", "1000"},
{"H7", "Minimum_voltage", "1000"},
{"H8", "Maximum_voltage", "1000"},
{"H9", "Last_full_charge", "3600"},
{"H10", "Num_automatic_sync", "0"},
{"H11", "Num_low_volt_alarms", "0"},
{"H12", "Num_high_volt_alarms", "0"},
{"H13", "Num_low_aux_vol_alarms", "0"},
{"H14", "Num_high_aux_vol_alarms", "0"},
{"H15", "Min_aux_volt", "1000"},
{"H16", "Max_aux_volt", "1000"},
{"H17", "Amount_discharged_energy", "100"},
{"H18", "Amount_charged_energy", "100"},
{"H19", "total_kWh", "100"},
{"H20", "today_kWh", "100"},
{"H21", "Max_pow_today", "0"},
//{"H22", "Yesterday_kWh", "100"},
//{"H23", "Max_pow_yesterday", "0"},
{"ERR", "Current_error", "0"}, // ERROR CODES SIEHE UNTEN
{"CS", "Operation_state", "0"},
{"BMV", "Model_description", ""},
//{"FW", "Firmware_version_16", ""},
//{"FWE", "Firmware_version_24", ""},
//{"PID","Device_model",""}, 
//{"SER#","Serial_number",""}, // deaktiviert
{"HSDS","Day","0"},
{"MODE","Device_mode",""},
{"AC_OUT_V","AC_out_volt","100"},
{"AC_OUT_I","AC_out_current","10"},
{"AC_OUT_S","AC_out_apparent_pow",""},
{"WARN","Warning_reason",""},
{"MPPT","Tracker_operation_mode",""},
{"MON","DC_monitor_mode",""},
{"DC_IN_V","DC_input_voltage","100"},
{"DC_IN_I","DC_input_current","10"},
{"DC_IN_P","DC_input_power","0"},
};


/* 
Off_reason-Codes LASTAUSGANG  und ihre Bedeutung

Code	Bedeutung
0	Keine Abschaltung (Load ist aktiv)
1	Manuell deaktiviert
2	Batterie zu leer
3	Batterie zu voll
4	Übertemperatur
5	Schutzmechanismus aktiv
6	Spannung zu hoch
7	Spannung zu niedrig
8	Batterie-Spannung zu niedrig
9	Batterie-Spannung zu hoch
10	Gerät startet gerade
11	Firmware-Update läuft
12	Kommunikationsfehler
13	Unbekannter Fehler

*/


/*

Liste typischer ERR-Fehlercodes und ihre Bedeutung
Code	Bedeutung
0	Kein Fehler
2	Gerät überhitzt
17	Solarladegerät überhitzt
18	Eingangsspannung zu hoch
19	PV-Spannung zu hoch
20	Batterie-Spannung zu hoch
21	Batterie-Spannung zu niedrig
33	Kommunikationsfehler
34	Interner Fehler
38	Hardwarefehler
116	Firmware-Update läuft


*/