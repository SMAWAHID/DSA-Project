<div align="center">

  <h1>🗺️ FAST NUCES Navigation System</h1>
  
  <p>
    <b>An intelligent indoor navigation system empowering independent wayfinding across campus.</b>
  </p>

  <p>
    <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599C?style=for-the-badge&logo=c%2B%2B" alt="C++" />
    <img src="https://img.shields.io/badge/Framework-Qt_5%2F6-41CD52?style=for-the-badge&logo=qt" alt="Qt" />
    <img src="https://img.shields.io/badge/Algorithm-Dijkstra-FF6F00?style=for-the-badge" alt="Algorithm" />
    <img src="https://img.shields.io/badge/License-MIT-blue?style=for-the-badge" alt="License" />
  </p>

  <h4>
    <a href="#-how-it-works">How It Works</a>
    <span> · </span>
    <a href="#-getting-started">Installation</a>
    <span> · </span>
    <a href="#-user-interface">Screenshots</a>
  </h4>
</div>

<br />

## 🎯 Overview

**FAST NUCES Navigation System** is a desktop application that calculates optimal routes across a university campus using Dijkstra's shortest path algorithm. The system visualizes **150+ location nodes** across **11 interactive floor maps** with real-time walking animation—helping anxious and introverted users navigate confidently without asking for directions.

> **Problem Solved:** Every semester, students feel anxious navigating unfamiliar campuses. This system gives them independence and eliminates first-week friction.

---

## ✨ Key Features

* ✅ **Intelligent Pathfinding** - Dijkstra's algorithm computes optimal routes in `<100ms`.
* ✅ **11 Interactive Floor Maps** - Color-coded by building (EE/CS/Multipurpose + Outdoor).
* ✅ **Real-Time Animation** - A red person icon guides you step-by-step along the path.
* ✅ **Multi-Level Support** - Automatically switches tabs between floors during navigation.
* ✅ **Intermediate Waypoints** - Route via optional intermediate locations using the "Via" feature.
* ✅ **Hierarchical Filtering** - Organized selection: `Building → Floor → Room`.
* ✅ **Visual Highlighting** - Hallways and paths highlight in red as you navigate.
* ✅ **Scalable Data** - CSV-based backend allows easy addition of new rooms/buildings.
* ✅ **Accessibility Focused** - Designed specifically for introverts and anxious users.

---

## 📊 By The Numbers

| Metric | Value |
| :--- | :--- |
| **Total Nodes** (Rooms/Locations) | **150+** |
| **Total Edges** (Hallway Connections) | **200+** |
| **Interactive Floor Maps** | **11** |
| **Buildings Covered** | **4** |
| **Path Calculation Speed** | **<100ms** |
| **Memory Footprint** | **<5MB** |
| **Floors Supported** | EE (5), CS (2), Multi (3), Outdoor (1) |

---

## 🛠️ Tech Stack

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Language** | C++11 / C++17 | Core logic and memory management. |
| **GUI Framework** | Qt 5 / 6 | Used for MainWindow, GraphicsView, and Scene. |
| **Core Algorithm** | Dijkstra | Priority-queue based shortest path finding. |
| **Data Structure** | Adjacency List | Weighted graph implementation + Tree Hierarchy. |
| **Visualization** | QGraphicsScene | Manages 2D items (nodes, edges, animations). |
| **Data Format** | CSV | Human-editable map data storage. |
| **Build System** | CMake 3.16+ | Cross-platform build configuration. |

---

## 🚀 Getting Started

### Prerequisites
* **Qt Creator 6.0** or higher (with Desktop kit)
* **C++ Compiler** (GCC, Clang, or MSVC)
* **CMake 3.16+**
* Git

### Installation

**Step 1: Clone the Repository**
```bash
git clone [https://github.com/YourUsername/FAST-NUCES-Navigation.git](https://github.com/YourUsername/FAST-NUCES-Navigation.git)
cd FAST-NUCES-Navigation
