# Hobby_Tracker — Client Side

This repository contains the client application for the **Hobby_Tracker** project. It is the frontend part of the system, responsible for the graphical interface and communicating with the server via API requests to fetch and synchronize data.

API: https://github.com/griy-rerere/hobby-tracker-api

## 📊 Graphical User Interface (GUI)

The application displays detailed user statistics, which include:

* Tracking and separating session times;

* Sorting data by various parameters;

* Visual charts and other informational graphical elements.

## 🔐 Registration and Authorization (JWT)

The project implements a user authentication system based on **JWT (Access/Refresh)**:

* **Registration:** The user creates a login and password via the application's GUI forms. The data is sent to the server, where the account is assigned a unique ID for subsequent validation upon login.

* **Authorization & Requests:** Upon logging in, the client receives JWT tokens. To access protected data, the application automatically attaches the token to the headers of HTTP requests.

* **Session Refresh:** The process of validating and refreshing expired tokens (Refresh) occurs automatically on the client side, completely invisible to the user.

## 🛠 Technology Stack and Dependencies

* **Programming Language:** C++

* **Framework:** Qt (including Network and GUI modules)

* **Build System:** CMake

* **IDE:** Qt Creator

## 🚀 Installation and Build (Windows)

To build the project on Windows, the **Qt Creator** IDE is used. All necessary tools for compilation and network requests are already included in the Qt framework.

### Step 1. Environment Preparation

To get started, you need to install the IDE and compiler :

1. Download the online **Qt** framework installer from the official website https://www.qt.io/development/download.(use VPN for open website and dowland)

2. Open the application and log into your Qt account.

3. During the installation process, select **Qt Creator** and the necessary components for work (e.g., MinGW or MSVC compiler, and CMake if it is not already installed on your system).

### Step 2. Cloning the Project

Clone this repository with the source code:

```bash
git clone https://github.com/Zer0Dash/Front_Hobby_Tracker
cd Front_Hobby_Tracker
```

### Step 3. Configuring and Building in Qt Creator

1. Open **Qt Creator** and click **"File" -> "Open File or Project..."**.

2. Select the `CMakeLists.txt` file located in the root of the `Desktop_Hobby_Tracker` folder.

3. The "Configure Project" window will appear. Select the desired Kit that matches your installed compiler (e.g., Desktop Qt MinGW or MSVC).

4. Click **"Configure Project"**.

5. After successful configuration, click the **Build** button (the hammer icon in the bottom left corner or `Ctrl+B`) to compile the project.

6. To launch the application, click the **Run** button (the green triangle or `Ctrl+R`).
