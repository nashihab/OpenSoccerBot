# OpenSoccerBot Web

Zero-build interactive documentation and engineering visualizer for the OpenSoccerBot manual RC soccer robot platform.

## Run locally

No npm, bundler or package manager is required.

From the repository root:

```bash
python -m http.server 8000
```

Open `http://localhost:8000/web/`.

The site is plain HTML/CSS/JavaScript so it can also be hosted directly from GitHub Pages.

## Included visualizers

- BASIC Arduino Uno / 2WD and ADVANCED ESP32 / 4WD mecanum switching
- Interactive chassis visualization
- Control-chain architecture
- Wiring explorer using the project's real SVG wiring diagrams
- Battery series/parallel calculator and TB6612 4S warning
- RC stick and differential/mecanum motor response visualizer
- Component explorer derived from the project BOM
- Firmware behavior summaries and direct source links
- Build sequence

## Source of truth

Firmware, BOM, wiring diagrams and detailed build documents remain beside this web application in the repository.

## License

MIT. See the repository `LICENSE` file.
