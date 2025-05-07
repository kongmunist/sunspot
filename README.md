# Sunspot: Solar-Powered LoRa Camera System

Sunspot is a sticker-based camera system that uses LoRa technology to transmit images to the web, powered entirely by solar energy. The system operates without the need for battery charging, making it a truly autonomous monitoring solution.

## Key Features

- **Solar-Powered Operation**: Runs entirely on harvested solar energy
- **LoRa Connectivity**: Long-range, low-power wireless communication
- **Sticker Form Factor**: Ultra-thin, flexible design for easy deployment
- **Zero Maintenance**: No battery charging required
- **Web Integration**: Images automatically uploaded to web dashboard

## Applications

- Environmental monitoring
- Wildlife tracking
- Remote surveillance
- Agricultural monitoring
- Infrastructure inspection

## Project Structure

```
sunspot/
├── firmware/
│   ├── CameraWebServer/    # Camera web server implementation
│   └── jpeg_stream_viewer.py  # JPEG stream viewer utility
└── sunspotv1/             # KiCad PCB design files
    ├── sunspotv1.kicad_sch    # Schematic file
    ├── sunspotv1.kicad_pcb    # PCB layout file
    └── sunspotv1.kicad_pro    # Project file
```

## Hardware

The hardware design is implemented in KiCad and includes:
- Custom PCB design (sunspotv1)
- Schematic and PCB layout files
- Project configuration

## Firmware

The firmware component includes:
- Camera web server implementation
- JPEG stream viewer utility for camera feed visualization