# 🐧 Pengu Engine
![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey?style=for-the-badge&logo=windows)
![Visual Studio](https://img.shields.io/badge/IDE-Visual%20Studio%202022-purple?style=for-the-badge&logo=visualstudio)
![Conan](https://img.shields.io/badge/package%20manager-Conan%202-orange?style=for-the-badge)
![Python](https://img.shields.io/badge/python-3.x-yellow?style=for-the-badge&logo=python)

A lightweight, extensible game engine built for rapid scene development and scripting.

[Getting Started](#-getting-started) · [Building](#️-building-the-project) · [Engine Guide](#-how-to-use-the-engine) · [Docs](./docs/)

---

## 📋 Overview

Pengu Engine is a game engine designed for ease of use — from scene creation and scripting to rendering. Whether you're prototyping a game or building something from the ground up, Pengu provides a streamlined workflow backed by a Visual Studio 2022 solution.

---

## ✅ Getting Started

### Prerequisites

Before building the project, make sure the following are installed on your machine.

| Requirement | Version | Notes |
|---|---|---|
| [Conan](https://conan.io/) | 2.x | Dependency manager |
| [Visual Studio](https://visualstudio.microsoft.com/) | 2022 | C++ workload required |
| [Python](https://www.python.org/) | 3.x | Required for Python tooling |

> **Already have all prerequisites?** Skip ahead to [Building the Project](#️-building-the-project).
>
> **Need to install Conan 2?** Follow the step-by-step guide: [Install Conan 2](./docs/ConanInstall.md)

### Setting Up the Python Environment

The `tools/Python/` folder contains Python-based tooling scripts. Before using them, set up the Python environment using the provided `requirements.txt`:
```cmd
python -m venv myenv
myenv\Scripts\activate
pip install -r requirements.txt
```

> ⚠️ Make sure to activate the environment before running any Python tools in `tools/Python/`.

---

## 🛠️ Building the Project

### 1. Locate the build script

Find `genproject.bat` inside the `tools/Project/` folder at the root of the repository:
```
pengu-engine/
└── tools/
    ├── Project/
    │   └── genproject.bat   ← run this
    └── Python/
        └── ...              ← Python tooling scripts
```

> ⚠️ If `genproject.bat` is missing, contact the engine maintainer to obtain a complete copy of the engine with all tooling included.

### 2. Run the script

Double-click `genproject.bat`, or run it from the command line:
```cmd
tools\Project\genproject.bat
```

### 3. Open in Visual Studio

The script will:
- Install all required dependencies via **Conan 2**
- Generate a Visual Studio solution (`.sln`) in the `build/` directory

Open the generated `.sln` file in **Visual Studio 2022** to begin development.

---

## 📖 How to Use the Engine

For a full step-by-step guide covering scene creation, scripting, rendering, and more, refer to the engine documentation:

**[→ Engine Guide](./docs/EngineGuide.md)**

---

## 📁 Project Structure
```
pengu-engine/
├── assets/                 # Engine assets
├── docs/
│   ├── ConanInstall.md     # Conan 2 installation guide
│   └── EngineGuide.md      # Full engine usage guide
├── examples/               # Example projects
├── include/                # Header files
├── lib/                    # Library files
├── src/                    # Source code
├── tools/
│   ├── Project/            # Project generation scripts
│   │   └── genproject.bat  # Generates the Visual Studio solution
│   └── Python/             # Python tooling scripts
├── premake5.lua            # Build configuration
├── requirements.txt        # Python dependencies for tools/Python/
└── build/                  # Generated Visual Studio solution (after build)
```

---

## 🤝 Contributing

If you'd like to contribute or report an issue, please reach out to the engine maintainer or open an issue in the repository.

---

## 📬 Contact

Have a question, found a bug, or want to get in touch with the team?

| Name | Type | Email |
|---|---|---|
| Kaelin Lind | Academic | [lind@esat-alumni.com](mailto:lind@esat-alumni.com) |
| Kaelin Lind | Personal | [kaelinlind@gmail.com](mailto:kaelinlind@gmail.com) |

---

*Made with ❄️ by the Pengu Engine team*