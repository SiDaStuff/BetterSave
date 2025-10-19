# About BetterSave

**BetterSave** is a comprehensive cloud save management system for Geometry Dash, created by **sidastuff**.

---

## 🎯 What is BetterSave?

BetterSave provides a secure, cloud-based solution for backing up, restoring, and syncing your Geometry Dash save data across multiple devices. Never lose your progress again!

### Core Features

- **☁️ Cloud Backup**: Upload your entire game save to Firebase cloud storage
- **🔐 Secure Login**: Firebase Authentication ensures your data is protected
- **💾 Quick Restore**: Download your saves from any device where you're logged in
- **🚀 Fast & Reliable**: Parallel chunked uploads with real-time progress tracking
- **📊 Transparent**: Built-in logging system shows you exactly what's happening
- **🔒 Safe**: Multiple protections to prevent data corruption or loss

---

## 👨‍💻 Developer

**Created by: sidastuff**

BetterSave is independently developed and maintained by sidastuff as a passion project to help the Geometry Dash community protect their save data.

---

## 🔧 Technical Overview

### Technology Stack

- **Framework**: Geode SDK for Geometry Dash modding
- **UI**: Cocos2d-x (Geometry Dash's native UI framework)
- **Backend**: Firebase Realtime Database + Firebase Authentication (REST API)
- **Encoding**: Hex encoding for binary save file conversion
- **Language**: C++20

### How It Works

1. **Authentication**: Users create an account with email/password via Firebase Auth
2. **Encoding**: Binary save files (`CCGameManager.dat` and `CCLocalLevels.dat`) are hex-encoded
3. **Chunking**: Large files are split into 200KB chunks for reliable uploads
4. **Storage**: Chunks are uploaded to Firebase Realtime Database in parallel
5. **Retrieval**: Downloads reassemble chunks in the correct order
6. **Decoding**: Hex data is decoded back to binary format
7. **Restoration**: Files are written to disk and the game restarts automatically

### Data Structure

```
Firebase Realtime Database:
└── users/
    └── {userId}/
        ├── saveData/
        │   └── meta (chunk counts, timestamp)
        └── chunks/
            ├── gm0, gm1, gm2... (GameManager data)
            └── ll0, ll1, ll2... (LocalLevels data)
```

---

## ✨ Features in Detail

### 🔐 Firebase Authentication
- Secure email/password authentication
- Persistent login with local credential storage
- Auto-login on subsequent game launches
- Safe logout with credential cleanup

### ☁️ Cloud Backup
- Hex encoding for reliable text-based storage
- Parallel chunked uploads (up to 10x faster than sequential)
- Real-time progress tracking with visual feedback
- Comprehensive error handling and retry logic

### 💾 Safe Restore
- Automatic local backups before downloading
- Data integrity verification
- Safe game restart using Geode's built-in utility
- Progress window prevents accidental interruption

### 📊 Activity Logging
- JSON-based structured logging
- All operations logged with timestamps
- Easy-to-read logs viewer popup
- Persistent storage for debugging

---

## 🛡️ Safety Features

BetterSave includes multiple layers of protection:

- ✅ **Window Locking**: Can't close during critical operations
- ✅ **Automatic Backups**: Local backups created before downloads
- ✅ **Error Recovery**: All error paths properly handled
- ✅ **Data Validation**: Files verified before upload/download
- ✅ **Safe Restart**: Uses Geode's crash-safe restart function
- ✅ **Comprehensive Logging**: Every action is logged for transparency

---

## 📈 Version History

### v1.0.0 (Current)
- ✅ Firebase Authentication (login/signup)
- ✅ Cloud save upload with parallel chunking
- ✅ Cloud save download with automatic game restart
- ✅ Persistent login with local credential storage
- ✅ Built-in logging system with viewer
- ✅ Progress tracking popup
- ✅ Hex encoding for binary data
- ✅ Comprehensive error handling
- ✅ Cross-device synchronization
- ✅ Safe window management

---

## 🌟 Why BetterSave?

### The Problem
Geometry Dash saves are stored locally, which means:
- ❌ Lost progress if you uninstall or lose your device
- ❌ Can't easily transfer saves between devices
- ❌ No built-in backup system
- ❌ Difficult to recover from file corruption

### The Solution
BetterSave provides:
- ✅ Automatic cloud backups with one click
- ✅ Access your saves from any device
- ✅ Built-in backup and restore functionality
- ✅ Safe, encrypted cloud storage via Firebase
- ✅ Complete transparency with activity logs

---

## 🎮 Supported Platforms

- ✅ **Windows** (Primary platform)
- ✅ **Mac** (Planned - Firebase ready, needs testing)

Current version is fully tested on Windows. Other platforms use the same Firebase backend but may need minor adjustments for file system paths.

---

## 📜 License & Credits

### Copyright
Copyright © 2024 **sidastuff**. All rights reserved.

### Credits
- **sidastuff** - Creator, Lead Developer, and Maintainer
- **Geode SDK Team** - For the incredible modding framework
- **Firebase** - For reliable cloud infrastructure
- **RobTop** - For creating Geometry Dash

### Third-Party Libraries
- Geode SDK (v4.9.0)
- Firebase REST API
- matjson (JSON library)
- fmt (String formatting)

---

## 🔗 Links

- **GitHub**: [github.com/sidastuff/BetterSave](https://github.com/sidastuff/BetterSave)
- **Geode Discord**: [discord.gg/geode](https://discord.gg/geode)
- **Geode SDK**: [geode-sdk.org](https://geode-sdk.org)

---

## ⚠️ Disclaimer

BetterSave is a third-party mod and is not affiliated with, endorsed by, or associated with RobTop Games or Geometry Dash. This mod is provided "as-is" for personal use. While BetterSave is designed with multiple safety features to protect your data, the developer (**sidastuff**) is not responsible for any data loss that may occur. Always maintain personal backups of your important save files.

---

## 🚀 Future Plans

Potential features for future versions:
- Multiple save slots (save snapshots with different names)
- Save sharing (share your progress with friends)
- Automatic periodic backups
- Desktop application for save management
- Save file comparison and merge tools
- Mobile platform support
- Cross-platform save migration

---

**Made with ❤️ by sidastuff**

*Thank you for using BetterSave! Your save files are important, and this mod exists to help protect them.*
