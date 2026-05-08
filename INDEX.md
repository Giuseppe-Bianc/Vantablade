---
aliases:
  - Home
tags:
  - docs
  - obsidian
  - vantablade
---

# Vantablade Vault

This repository root is the Obsidian vault for Vantablade. Use this note as the map of content when you open the project in Obsidian.

## Start Here

- [[Project Overview]]
- [[Architecture]]
- [[Development Setup]]

## Build and Tooling

- [[Build Tooling]]
- [[Dependencies]]
- [[Testing]]

## Code Orientation

- [[Code Map]]
- `src/vantablade_Core_lib/` contains the foundational utilities layer.
- `src/vantablade_lib/` contains the Vulkan and application runtime layer.
- `src/vantablade/` contains the CLI executable entry point.

## Repository Landmarks

- `CMakeLists.txt` defines the top-level project, packaging, and optional test and fuzz targets.
- `CMakePresets.json` defines cross-platform configure and test presets.
- `Dependencies.cmake` pulls third-party libraries through CPM.
- `shaders/` contains GLSL sources compiled into SPIR-V during the build.
- `docs/` contains the tracked project notes for this vault.

## Using This Vault

- Start in Graph view from this note to visualize the major documentation clusters.
- Use Backlinks on notes like [[Architecture]] and [[Code Map]] while reading source.
- Keep new notes in `docs/` so vault content stays tracked with the repository.

Last updated: 2026-05-08
