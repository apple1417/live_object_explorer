# Live Object Explorer
Modding/reverse engineering tool for Unreal Engine games. Displays an object's properties in
interactive, searchable windows, and lets you edit them, or watch the game edit them, live.

![demo image](https://github.com/user-attachments/assets/9a149157-f7ec-46b4-bcbf-d8fef12e350a)

Based on [unrealsdk](https://github.com/bl-sdk/unrealsdk), supports everything it does.

# Installation & Usage

1. Install unrealsdk. Easiest done by installing the [SDK Mod Manager](https://bl-sdk.github.io/)
   for your game.

2. Download the relevant release for your game. Note each release mentions which unrealsdk versions
   it's compatible for. Extract it into your plugins folder.

3. In game, run the `explore` command. This command supports the following usages:
   - `explore`: Just opens the gui.

   - `explore SomeClassName`: Opens the gui, and searches for all objects of the given class.

   - `explore Some.Obj.Path`: Opens the gui, and dumps the given object.
     
     Note that if this resolves to a class object, it searches for all objects of that class. To
     dump a class object instead, search for `Class`, and select it from the results list.


If needed, you can adjust a few settings via the `unrealsdk.toml`. The default settings should work
fine in most cases. See [`supported_settings.toml`](supported_settings.toml) for more.
