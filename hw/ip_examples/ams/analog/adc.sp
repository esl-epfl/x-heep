** Copyright EPFL contributors.
** Licensed under the Apache License, Version 2.0, see LICENSE for details.
** SPDX-License-Identifier: Apache-2.0

** Uses the 65nm_bulk PTM Bulk CMOS model, February 22, 2006 release
** (obtained from https://ptm.asu.edu)

.include "../../../hw/ip_examples/ams/analog/65nm_bulk.pm"

v_vdd VDD 0 1.2
v_gnd GND 0 0
.global VDD GND

.subckt INVERTER GND IN OUT VDD
MN0 OUT IN GND GND nmos L=60n W=400n
MN1 OUT IN VDD VDD pmos L=60n W=400n
.ends INVERTER

.subckt DEMUX A B C D GND SEL_0 SEL_1 VDD
MN20 B SEL_0_not GND GND  nmos L=60n W=200n
MN18 B SEL_1 GND GND nmos L=60n W=200n
MN17 C SEL_1_not GND GND nmos L=60n W=200n
MN15 C SEL_0 GND GND nmos L=60n W=200n
MN9 A SEL_1 GND GND nmos L=60n W=200n
MN7 A SEL_0 GND GND nmos L=60n W=200n
MN13 D SEL_1_not GND GND nmos L=60n W=200n
MN11 D SEL_0_not GND GND nmos L=60n W=200n
XI4 GND SEL_1 SEL_1_not VDD INVERTER
XI3 GND SEL_0 SEL_0_not VDD INVERTER
MN21 B SEL_0_not net25 VDD pmos L=60n W=800n
MN19 net25 SEL_1 VDD VDD pmos L=60n W=800n
MN16 net26 SEL_1_not VDD VDD pmos L=60n W=800n
MN14 C SEL_0 net26 VDD pmos L=60n W=800n
MN8 net28 SEL_1 VDD VDD pmos L=60n W=800n
MN6 A SEL_0 net28 VDD pmos L=60n W=800n
MN10 D SEL_0_not net27 VDD pmos L=60n W=800n
MN12 net27 SEL_1_not VDD VDD pmos L=60n W=800n
.ends DEMUX

.subckt ANALOG_MUX A B C D GND OUT SEL_0 SEL_1 VDD
XI0 SEL_A SEL_B SEL_C SEL_D GND SEL_0 SEL_1 VDD DEMUX
XI4 GND SEL_C SEL_C_not VDD INVERTER
XI3 GND SEL_D SEL_D_not VDD INVERTER
XI1 GND SEL_A SEL_A_not VDD INVERTER
XI2 GND SEL_B SEL_B_not VDD INVERTER
MN4 OUT SEL_D_not D VDD pmos L=60n W=2u
MN2 OUT SEL_B_not B VDD pmos L=60n W=2u
MN0 OUT SEL_A_not A VDD pmos L=60n W=2u
MN21 OUT SEL_C_not C VDD pmos L=60n W=2u
MN5 D SEL_D OUT GND nmos L=60n W=1u
MN3 B SEL_B OUT GND nmos L=60n W=1u
MN1 A SEL_A OUT GND nmos L=60n W=1u
MN20 C SEL_C OUT GND nmos L=60n W=1u
.ends ANALOG_MUX

.subckt COMPARATOR GND IN_N IN_P OUT VDD
MN52 OUT net54 VDD VDD pmos L=60n W=1.2u
MN49 net52 net29 net31 VDD pmos L=60n W=1.2u
MN48 net54 net24 net31 VDD pmos L=60n W=1.2u
MN34 net15 net15 VDD VDD pmos L=60n W=1.2u
MN39 net29 net51 VDD VDD pmos L=60n W=1.2u
MN38 net51 net51 VDD VDD pmos L=60n W=1.2u
MN37 net21 IN_N net19 VDD pmos L=60n W=1.2u
MN36 net24 net15 VDD VDD pmos L=60n W=1.2u
MN35 net13 IN_P net19 VDD pmos L=60n W=1.2u
MN53 OUT net54 GND GND nmos L=60n W=600n
MN51 net54 net52 GND GND nmos L=60n W=600n
MN50 net52 net52 GND GND nmos L=60n W=600n
MN47 net29 net29 GND GND nmos L=60n W=600n
MN46 net24 net24 GND GND nmos L=60n W=600n
MN45 net29 net24 GND GND nmos L=60n W=600n
MN41 net21 net21 GND GND nmos L=60n W=600n
MN40 net13 net13 GND GND nmos L=60n W=600n
MN44 net24 net29 GND GND nmos L=60n W=600n
MN33 net51 IN_N net18 GND nmos L=60n W=600n
MN32 net15 IN_P net18 GND nmos L=60n W=600n
MN43 net15 net21 GND GND nmos L=60n W=600n
MN42 net51 net13 GND GND nmos L=60n W=600n
I2 VDD net31 DC=200u
I1 net18 GND DC=100u
I0 VDD net19 DC=200u
.ends COMPARATOR

.subckt AMS_ADC_1b GND OUT SEL<1> SEL<0> VDD
XI0 net010 net7 net8 net02 GND Vmux_out SEL<0> SEL<1> VDD ANALOG_MUX
XI1 GND Vmux_out Vin OUT VDD COMPARATOR
R4 net010 GND 100k
R3 net7 net010 100k
R2 net8 net7 100k
R1 net02 net8 100k
R0 VDD net02 100k
V2 Vin GND SIN 600m 600m 1e6 250e-9
.ends AMS_ADC_1b

