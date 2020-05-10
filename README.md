# Autonomous vehicle tutored project

## Important

This git repository contains submodules (references to other included git repositories, learn more : https://git-scm.com/book/en/v2/Git-Tools-Submodules). As a consequence, **in order to clone this repository you will have to use the following command** :

    git clone --recurse-submodules https://github.com/jb332/autonomous_vehicle_pir.git

If you already cloned this repository the classic way, you will get blank directories instead of submodules, don't worry, execute the following command :

    git submodule update --init --recursive

## Conception
You can find diagrams to understand how the project works in the following directory :

    diagrams/

The use case diagram describes the scope of our project, and the global diagram describes in detail how the different elements of the system (Raspberry Pi, Server, Client, etc.) work and the protocols they use (OM2M, ROS, HTTP, etc.)
There are also sequences diagrams describing some features.

## Implementation

The project has been divided into two main modules :

 - Web site (with an andoid web app)
 - PID regulator (with a simulator)

### Web site

The web site is the interface the user will manipulate to select a stop and be transported by the shuttle. You can find more details in the web site submodule README file located in the following directory  :

    implementation_web/

There is a second way to use the service, using an android application that runs a modified version of the web site as its main view, its source code is available in the following directory :

    implementation_android/

### PID regulator

The PID regulator listens to the vehicle sensors and send appropriate commands to compensate the vehicle angle error towards its destination. Consequently, the vehicle follow a rather linear trajectory towards its destination. To anticipate the steering angle when the destination is changed, intermediary destination points are added and targeted by the regulator.

In the current context, we could nor reach the autonomous shuttle, thus a vehicle simulator can be run concomitantly with the PID. It listens for commands emitted by the latter via OM2M and sends simulated sensors information. It shows the vehicle movement on a window.

There are 3 versions of the PID / simulator :
 - Python / Gtk+ (via PyGObject)
 - C / Gtk+ / GObject / Glib (**this version includes neither mutex nor proper thread termination**)
 - C / OpenGL / pthread

Their source code is available in the following directory :

    implementation_pid/pid_and_simulator/

You will find a README file for each version.

*Note : We recommend that you use either the first version or the third.*

## Project report

The project reports are available on the overleaf platform :
[https://www.overleaf.com/project/5ea6e5b4416f0b00017643cc](Technical accomplishment )
[https://www.overleaf.com/8927888864mqkshqjygyjf](State of the art )

## Other documents and drafts

We used Google Drive at the beginning of the project, so you can find drafts and other documents here :
[https://drive.google.com/drive/folders/1lNKAHbNH-b6xcy80rTQNm9o_j-fEMg1S](https://drive.google.com/drive/folders/1lNKAHbNH-b6xcy80rTQNm9o_j-fEMg1S)


