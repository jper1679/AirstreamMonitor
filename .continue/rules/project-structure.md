## Structure du projet AirstreamMonitor

- Le code est dans `src/` and related include directories.
- Le point d’entrée principal est dans `src/AirstreamMonitor.cpp`.
- Les modules UI sont dans `src/ui/`, avec LVGL.
- Les drivers hardware sont dans `src/hardware/`.
- Il y a des fichiers importants dans `src/include/` et `src/lib/`.
- Les fichiers C++ utilisent l’extension `.cpp` ou `.h`.
- Continue doit **jamais** répondre comme si le projet était en Python; s’il ne trouve pas de fichier C++ cohérent, il doit le dire.