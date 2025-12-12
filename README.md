
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
````

**Step 2: Create Build Directory**

```bash
mkdir build && cd build
```

**Step 3: Build the Project**

```bash
cmake ..
make
# Or use 'cmake --build .' on Windows
```

**Step 4: Run the Application**

```bash
./FAST-NUCES-Navigation
```

*Alternatively, open `CMakeLists.txt` in Qt Creator and click **Run**.*

-----

## 📖 How to Use

### Basic Navigation

1.  **Select Starting Location**
      * Choose building from "Source Area".
      * Select specific room from "Location".
2.  **[Optional] Add Intermediate Stop**
      * Choose "Via Area" and location if you want to stop somewhere in between.
3.  **Select Destination**
      * Choose target building and room from "Dest Area".
4.  **Find Route**
      * Click **Search**.
      * The map will highlight the path, and the red dot will animate your walk.
      * Text instructions will appear in the sidebar.

### Controls

  * **Zoom:** Mouse scroll wheel.
  * **Pan:** Click and drag on the map.
  * **Switch Floors:** Click the tabs at the top (e.g., `EE Building` → `EE-A`, `EE-B`).
  * **Global View:** Click the `Outdoor Map` tab.

-----

## 🎨 User Interface

### Main Window Layout

\<img width="100%" alt="Main Window Interface" src="https://github.com/user-attachments/assets/7f8dc94a-8492-47ef-85ce-1847644c6475" /\>

### Floor Layout & Visualization

[Image of detailed indoor floor map]

\<img width="80%" alt="Floor Layout" src="https://github.com/user-attachments/assets/07dae5b1-a8f1-4f30-bf69-0831a803dbaf" /\>

-----

## 🧠 How It Works

### 1\. Data Layer (CSV)

The system loads `data/campus_map_detailed.csv` on startup.

```csv
SECTION 1: NODES
EE-Lab-1, 120, 100  <-- Node ID, X, Y

SECTION 2: EDGES
EE-Lab-1, EE-Hall-A, 15  <-- From, To, Weight (Distance)
```

### 2\. Graph Construction

  * Parses CSV to create **Nodes** with visual coordinates.
  * Builds an **Adjacency List** where every room knows its neighbors and the distance to them.

### 3\. Pathfinding Engine (Dijkstra)

1.  **Init:** Set Start distance to 0, all others to Infinity.
2.  **Priority Queue:** Explore the closest unexplored node.
3.  **Relax:** If a new path to a neighbor is shorter, update the distance.
4.  **Reconstruct:** Trace breadcrumbs backwards from Destination to Start.

### 4\. Visualization & Animation

  * **QGraphicsScene:** Draws the static map layout.
  * **QSequentialAnimationGroup:** Creates a sequence of movements. The person icon moves from Node A to Node B, then B to C, automatically switching scenes if the floor changes.

-----

## 🏗️ Project Structure

```text
FAST-NUCES-Navigation/
├── CMakeLists.txt        # Build configuration
├── README.md             # Documentation
├── data/
│   └── campus_map.csv    # The map database
├── include/              # Header files
│   ├── core/             # CampusGis logic
│   ├── graph/            # Graph & Dijkstra algo
│   ├── gui/              # MainWindow & UI logic
│   └── trees/            # LocationTree hierarchy
├── src/                  # Source files (.cpp)
└── resources/            # Images and QRC assets
```

-----

## ⚙️ Configuration

**Adding New Rooms:**
Simply edit `data/campus_map_detailed.csv`. You do not need to recompile C++ code to add simple nodes\!

```csv
New-Room-Name, 500, 300
New-Room-Name, Existing-Hallway, 10
```

**Customizing Colors:**
Colors are defined in `src/gui/MainWindow.cpp`. You can modify the hex codes in `drawFloorSchematic()` to match your branding.

-----

## 🚀 Future Roadmap

  * 📱 **Mobile Application** (iOS/Android port using Qt Quick).
  * 🔥 **Heatmaps** for real-time crowd density.
  * ♿ **Accessibility Mode** (Prioritize elevators over stairs).
  * 🌐 **Web Interface** (WASM build).
  * ⏱️ **ETA Estimation** based on average walking speed.

-----

## 🤝 Contributing

Contributions are welcome\! Please follow these steps:

1.  Fork the repository.
2.  Create your feature branch (`git checkout -b feature/AmazingFeature`).
3.  Commit your changes (`git commit -m 'Add AmazingFeature'`).
4.  Push to the branch.
5.  Open a Pull Request.

-----

## 📝 License

This project is licensed under the **MIT License**. See the `LICENSE` file for details.

-----

## 👨‍💻 Author

**Syed Mawahid Hussain**
& **Umais Ahmed**

  * 💼 LinkedIn: [linkedin.com/in/yourprofile](https://www.linkedin.com/in/syed-mawahid-hussain-ab951b180/)
  * 📧 Email: k241041@nu.edu.pk & k241003@nu.edu.pk
  * 🐙 GitHub: [@SMawahid](https://github.com/SMAWAHID)

-----

\<div align="center"\>
\<b\>Built with ❤️ for introverts, by an introvert.\</b\><br>
⭐ If you found this helpful, please give it a star\! ⭐
\</div\>

```
```
