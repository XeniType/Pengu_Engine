# Conan 2 Installation Guide

## Prerequisites

- Python 3.6 or higher
- pip (Python package manager)

Verify your Python installation:

```bash
python --version
pip --version
```

---

## Step 1: Install Conan 2

The recommended way to install Conan 2 is via pip:

```bash
pip install conan
```

To install a specific version of Conan 2:

```bash
pip install conan==2.x.x
```

Verify the installation:

```bash
conan --version
```

You should see output like `Conan version 2.x.x`.

> **Note:** If you have Conan 1.x already installed, upgrading will replace it. Conan 2 has breaking changes — see the [migration guide](https://docs.conan.io/en/2/installation.html) if you have existing projects.

---

## Step 2: Create the Default Profile

Conan uses **profiles** to describe your build environment (compiler, OS, architecture, etc.). Without a default profile, most commands will fail.

Run the following command to auto-detect your system and create the default profile:

```bash
conan profile detect
```

This creates a profile at:

- **Linux/macOS:** `~/.conan2/profiles/default`
- **Windows:** `C:\Users\<YourUser>\.conan2\profiles\default`

---

## Step 3: Create the `default_build` Profile

Some workflows (especially cross-compilation) require a separate `default_build` profile. Create it by copying the default profile:

**Linux/macOS:**

```bash
cp ~/.conan2/profiles/default ~/.conan2/profiles/default_build
```

**Windows (Command Prompt):**

```cmd
copy %USERPROFILE%\.conan2\profiles\default %USERPROFILE%\.conan2\profiles\default_build
```

**Windows (PowerShell):**

```powershell
Copy-Item "$env:USERPROFILE\.conan2\profiles\default" "$env:USERPROFILE\.conan2\profiles\default_build"
```

Alternatively, you can generate it directly with:

```bash
conan profile detect --name default_build
```

---

## Step 4: Verify Your Setup

List all available profiles:

```bash
conan profile list
```

You should see both `default` and `default_build` listed.

Inspect a profile's contents:

```bash
conan profile show default
conan profile show default_build
```

Example output:

```
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
build_type=Release
```

---

## Optional: Edit a Profile

You can manually edit a profile to customise settings:

```bash
conan profile path default        # shows the file path
```

Then open the file in any text editor and modify it. Common settings to adjust:

| Setting | Example Values |
|---|---|
| `build_type` | `Release`, `Debug` |
| `compiler` | `gcc`, `clang`, `msvc` |
| `compiler.version` | `11`, `12`, `14` |
| `arch` | `x86_64`, `armv8` |

---

## Quick Reference

| Command | Description |
|---|---|
| `pip install conan` | Install Conan 2 |
| `pip install --upgrade conan` | Upgrade to latest Conan 2 |
| `conan --version` | Check installed version |
| `conan profile detect` | Auto-create default profile |
| `conan profile detect --name default_build` | Create named profile |
| `conan profile list` | List all profiles |
| `conan profile show <name>` | Show profile contents |

---

## Troubleshooting

**`ERROR: Profile not found: default`**  
Run `conan profile detect` to generate the default profile.

**`ERROR: Profile not found: default_build`**  
Run `conan profile detect --name default_build` or copy your default profile (see Step 3).

**Conan 1.x is still being used after upgrade**  
You may have multiple Python environments. Try:
```bash
pip3 install --upgrade conan
python3 -m pip install --upgrade conan
```

---

*For more information, visit the [official Conan 2 documentation](https://docs.conan.io/en/2/).*