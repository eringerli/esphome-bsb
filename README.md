# BSB component for ESPHome

BSB is a protocol used by many heating systems. This is a simple and small (<1000 SLOC C++, <350 SLOC Python) implementation of BSB as an ESPHome component. It can be used together with other sensors and protocols like reading electricity, water or gas meters, which are usually in the same room as the heating system. Even multiple BSB busses are possible.

Use the documentation of [BSB-LAN](https://docs.bsb-lan.de/index.html) to get details, but the physical interface is really
simple: [schematics](https://github.com/fredlcore/BSB-LAN/blob/master/BSB_LAN/schematics/bsb_adapter.pdf). If you forego the galvanic isolation you can even cobble together an interface with some resistors and a transistor and inverting the GPIOs as needed. I personally use two small-signal solid state relays (SSR), these are basically opto-couplers with a MOSFET driver. Beware that the ESP32s can only handle 3.3V and that opto-couplers have a current dependent maximum transmission frequency, so be sure to look inside the datasheets of your specific chips.

## Field IDs
The used field IDs can be gleaned from: [BSB_LAN_custom_defs.h.default](https://github.com/fredlcore/BSB-LAN/blob/v2.2.2/BSB_LAN/BSB_LAN_custom_defs.h.default).

Yes, it is unnessesary hard to get them, but this comes from the undocumented, grown over decades of many, *many* different heating systems control units and therefore not logical structure of theses numbers. But there is a silver lining: if you set the parameters on the controlling unit on the heating system and listen at the same time on the bus, the IDs/packets get printed in the log on the `DEBUG` level. After some experimentation with the the data type and the factors, you can add almost any parameter to the YAML. Sadly, there is no apparent correlation between parameter number and field ID.

## Component
This component can be added as an external component, as shown in the example code below, so no need to clone/fork the repository.

```yaml
external_components:
 - source: github://eringerli/esphome-bsb
     refresh: 0s
     components: [bsb]
```

## Arduino/ESP-IDF
I had some problems with the arduino framework, especially with random reboots after a couple of hours, so I changed to ESP-IDF. The component is compatible with both, YMMV.

```yaml
esp32:
  framework:
    type: esp-idf
```

## UART bus
Depending on the physical interface, you have to invert the GPIOs. Keep the `baud_rate`, `data_bits`, `parity` and `stop_bits` exactly as below.

```yaml
uart:
  - id: uart_bsb
    rx_pin:
      number: GPIO02
      inverted: false
    tx_pin:
      number: GPIO03
      inverted: false
    baud_rate: 4800
    data_bits: 8
    parity: ODD
    stop_bits: 1
```

## BSB
| Key | Class | Default | Description |
| --- | --- | --- | --- |
| `retry_count` | optional | 3 | how many time to repeat trying to send a telegram |
| `retry_interval` | optional | 15s | what interval to wait for after `retry_count` retries |
| `query_interval` | optional | 0.25s | time between communications. Be aware that the heating system needs some time to process the request and send back data. 4Hz seems to be the sweet spot with my heating system. |
| `source_address` | optional | 66 | address to send from, usually 66 |
| `destination_address` | optional | 0 | address of the heating system, usually 0 |

```yaml
bsb:
  id: bsb1
  uart_id: uart_bsb
```

## General advice
Be sure to set the right `unit_of_measurement` (usually `°C`, `s` or `bar`), `accuracy_decimals` and `device_class` (usually `temperature`, `duration` or `pressure`). Also set the `mode` of the numbers to `box` if you want to set the parameters with increased accuracy. Use `factor` and `divisor` to calculate the actual value to send to the heating system, if you get strange values after setting a value and reading it back.

## Sensors
This is the main way to get data *out* of the heating system.

| Key | Class | Default | Description |
| --- | --- | --- | --- |
| `bsb_id` | required | | the BSB bus |
| `field_id` | required | | the uint32 of the field ID, pe `0x053D0000` |
| `type` | required | | the type of the parameter, one of `UINT8`, `INT8`, `INT16`, `INT32`, `TEMPERATURE` or `ROOMTEMPERATURE` |
| `parameter_number` | optional |  | this is not used currently, but it is good to document this number in the YAML. |
| `factor`, `divisor`| optional | 1 | either use filters or these two parameters to calculate the actual value to send to the frontend. `value = value_on_the_bus * factor / divisor` |
| `update_interval` | optional | 15min | interval to refresh the value from the heating system. Beware that reading a lot of data with an high update frequency can overload the heating system or the bus |
| `enable_byte`| optional | 1 | some parameters use a special enable byte, here it can be defined |

## Text Sensors
| Key | Class | Default | Description |
| --- | --- | --- | --- |
| `bsb_id` | required | | the BSB bus |
| `field_id` | required | | the uint32 of the field ID, pe `0x053D0001` |
| `parameter_number` | optional |  | this is not used currently, but it is good to document this number in the YAML. |
| `update_interval` | optional | 15min | interval to refresh the value from the heating system. Beware that reading a lot of data with an high update frequency can overload the heating system or the bus |

## Numbers
This is the main way to get data *into* the heating system.

| Key | Class | Default | Description |
| --- | --- | --- | --- |
| `bsb_id` | required | | the BSB bus |
| `field_id` | required | | the uint32 of the field ID, pe `0x053D0001` |
| `parameter_number` | optional |  | this is not used currently, but it is good to document this number in the YAML. |
| `update_interval` | optional | 15min | interval to refresh the value from the heating system. Beware that reading a lot of data with an high update frequency can overload the heating system or the bus |
| `factor`, `divisor`| optional | 1 | use these two parameters to calculate the actual value to send to the frontend. `value = value_on_the_bus * factor / divisor` |
| `enable_byte`| optional | 1 | some parameters use a special enable byte, here it can be defined |
| `broadcast` | optional | false |  to send as an INF telegram on the bus |
| `step` | required | | the step in the frontend |
| `min_value` | required | | the min value in the frontend |
| `max_value` | required | | the max value in the frontend |

### INF/Broadcast
Some values have to be sent as INF telegrams, like the room or the outside temperature. For my heating systems (and apparently many others too), you have to send the room temperature as an INF with the special type `ROOMTEMPERATURE`, but the outside temperature with the type `TEMPERATURE`. And INF telegrams don't get ack'ed from the heating system, so some experimentation is needed. 

This is the code I use in my heating system:
```yaml
number:
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x2D3D0215
    parameter_number: 10000
    enable_byte: 0x06
    type: roomtemperature
    broadcast: true
    name: Heating circuit 1 - room temperature
    update_interval: 60s
    unit_of_measurement: "°C"
    min_value: 0
    max_value: 35
    step: .01
    mode: box
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x2E3E0215
    parameter_number: 10001
    type: roomtemperature
    broadcast: true
    name: Heating circuit 2 - room temperature
    update_interval: 60s
    unit_of_measurement: "°C"
    min_value: 0
    max_value: 35
    step: .01
    mode: box
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x0500021F
    parameter_number: 10003
    type: temperature
    broadcast: true
    name: Outside temperature
    update_interval: 60s
    unit_of_measurement: "°C"
    min_value: -30
    max_value: 45
    step: .01
    mode: box
```

# Getting Started
You usually want to read out the identification and the type of the heating system, so you can search for the parameters in the header file from BSB-LAN.

Here is a snippet to start with:
```yaml
external_components:
 - source: github://eringerli/esphome-bsb
     refresh: 0s
     components: [bsb]

uart:
  - id: uart_bsb
    rx_pin:
      number: GPIO02
      inverted: false
    tx_pin:
      number: GPIO03
      inverted: false
    baud_rate: 4800
    data_bits: 8
    parity: ODD
    stop_bits: 1

bsb:
  id: bsb1
  uart_id: uart_bsb

sensor:
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x053D0000
    parameter_number: 6222
    type: int16
    name: Heating System Type
    update_interval: 10min
    unit_of_measurement: ""
    accuracy_decimals: 0

text_sensor:
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x053D0001
    parameter_number: 6224
    name: Unit Identification
    update_interval: 10min
```

## Full Example
Please add the other important bits, here are only the BSB specific definitions shown.

```yaml
esp32:
  framework:
    type: esp-idf

external_components:
 - source: github://eringerli/esphome-bsb
     refresh: 0s
     components: [bsb]

uart:
  - id: uart_bsb
    rx_pin:
      number: GPIO02
      inverted: false
    tx_pin:
      number: GPIO03
      inverted: false
    baud_rate: 4800
    data_bits: 8
    parity: ODD
    stop_bits: 1

bsb:
  id: bsb1
  uart_id: uart_bsb

sensor:
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x053D0000
    parameter_number: 6222
    type: int16
    name: Heating System Type
    update_interval: 10min
    unit_of_measurement: ""
    accuracy_decimals: 0

  - platform: bsb
    bsb_id: bsb1
    field_id: 0x0D3D0519
    parameter_number: 8310
    type: temperature
    name: Kesseltemperatur
    update_interval: 10s
    unit_of_measurement: "°C"
    accuracy_decimals: 2
    device_class: temperature

  - platform: bsb
    bsb_id: bsb1
    field_id: 0x053D0834
    parameter_number: 8326
    type: int8
    name: Brennermodulation
    update_interval: 10s
    unit_of_measurement: "%"
    accuracy_decimals: 2

  - platform: bsb
    bsb_id: bsb1
    field_id: 0x053D3063
    parameter_number: 8327
    type: int16
    name: Wasserdruck
    update_interval: 30min
    unit_of_measurement: "bar"
    accuracy_decimals: 3
    device_class: pressure
    divisor: 1000

text_sensor:
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x053D0001
    parameter_number: 6224
    name: Unit Identification
    update_interval: 10min

number:
  - platform: bsb
    bsb_id: bsb1
    field_id: 0x2D3D0215
    parameter_number: 10000
    # type: temperature
    type: roomtemperature
    broadcast: true
    name: Heizkreis 1 - Raumtemperatur
    update_interval: 60s
    unit_of_measurement: "°C"
    device_class: temperature
    min_value: 0
    max_value: 35
    step: .01
    mode: box

  - platform: bsb
    bsb_id: bsb1
    field_id: 0x2D3D05F6
    parameter_number: 720
    type: int16
    name: Heizkreis 1 - Kennlinie Steilheit
    update_interval: 30min
    unit_of_measurement: ""
    min_value: -10
    max_value: 10
    step: .01
    mode: box
    divisor: 50
```

