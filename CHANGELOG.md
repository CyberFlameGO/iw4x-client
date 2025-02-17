# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog v0.3.0](http://keepachangelog.com/en/0.3.0/) and this project adheres to [Semantic Versioning](http://semver.org/).

## [0.7.5] - 2022-09-03

### Added

- Add `bg_rocketJumpScale` Dvar (#413)
- Add `CastFloat` GSC function (#414)
- Add `Strtol` GSC function (#414)
- Add `bg_lean` Dvar (#421
- Add voice chat (#425)
- Add `vote` & `callvote` client commands (#447)
- Add `kill` client command (#451)
- Add `voteKick`, `voteTempBan`, `voteTypeMap`, `voteMap` and `voteGame` UI script tokens (#456)
- Add `Int64IsInt`, `Int64ToInt` and `Int64OP` GSC functions (#419)

### Changed

- Steam status is no longer set to busy (#417)
- `HttpGet`& `HttpCancel` are disabled for security reasons (#449)
- 'g_allowVote' is a replicated Dvar (#457)

### Fixed

- Fixed `startSingleplayer` command (#404)
- General stability update

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.4] - 2022-07-28

### Added

- Add `DisableWeaponPickup` GSC method (#329)
- Add `EnableWeaponPickup` GSC method (#329)
- Add `protect-saved-dvars` command line argument (#335)
- Add `clanName` dvar. Can be edited in the `barracks` menu (#361)
- Add DLC9 containing classic maps from CoD4: Backlot, Chinatown, Winter Crash, Pipeline and Downpour

### Changed

- `sv_mapRotationCurrent` functionality has been restored for backwards compatibility (#328)
- GSC IO Functions are restricted to the `scriptdata` folder (#322)
- `scriptablehttp` command line argument is no longer needed (#322)
- Removed `noclip` built-in GSC method. Replaced with script-based solution (#387)
- Removed `ufo` built-in GSC method. Replaced with script-based solution (#387)
- Removed `god` built-in GSC method. Replaced with script-based  solution (#388)
- Removed `demiGod` built-in GSC method. Replaced with script-based  solution (#388)
- Removed `noTarget` built-in GSC method. Replaced with script-based  solution (#388)
- Added to the iw4x-rawfiles `common_scripts\iw4x_utility` GSC script, it contains the scripts-based solution for the removed GSC built-in methods

### Fixed

- Node system works again for clients (#331)
- Fixed output of `g_log_list` & `log_list` (#336)
- Fixed toast notifications (#337)
- Fixed `banClient` command (#311)
- Fixed singleplayer maps crashing in renderer module (#340)
- Fixed muzzle flash on COD:OL maps (#352)
- Server commands are no longer being registered twice (#339)
- Fixed issue where grenades and projectiles had no bounce sounds (#368)
- Fixed issue that could cause the game to crash when loading CoD4 maps (#372)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.3] - 2022-06-23

### Added

- Add SetName GSC method (#288)
- Add ResetName GSC method (#288)
- Add OnPlayerSay GSC function (#265)
- Add Give client command (works with weapons only) (#292)
- Add `sv_disableChat` Dvar (#290)
- Add `addMap` command (#302)
- Add `addGametype` command (#302)

### Changed

- `openLink` command was removed for security reasons  (#286)
- `sv_mapRotationCurrent` is not being used anymore (#302)

### Fixed

- Fix font generation (#277)
- Fix crash on clearing key binds (#278)
- Fix maps dropping out of the map rotation when using `sv_randomMapRotation` (#283)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.
- Toast notifications have been disabled because they cause a crash that needs to be investigated

## [0.7.2] - 2022-05-10

### Added

- Add IsArray GSC function (#248)
- Keybind fields in menus work with controller keys (#255)

### Changed

- GSC function `GetSystemTime` now returns the Unix time (#258)

### Fixed

- Knife charge is fixed for controller players (#259)
- Fixed internet, local and favorites filters (#260)
- `sv_lanOnly` Dvar now prevents the server from sending heartbeats to master if set to true (#246)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.1] - 2022-05-03

### Added

- Add ToUpper GSC Function (#216)
- Add StrICmp GSC Function (#216)
- Add IsEndStr GSC Function (#216)
- Add DropAllBots GSC Function (#174)
- Add GSC entity field `entityflags` (#228)
- Add GSC client field `clientflags` (#228)
- Add bg_surfacePenetration Dvar (#241)
- Add bg_bulletRange Dvar (#241)

### Changed

- Test clients' native functionality has been restored by default (#162)
- Renamed GSC method `isBot` to `IsTestClient` (#162)
- Custom GSC functions can be called correctly from a game script (#216)
- GSC functions `HttpCancel` and `HttpCancel` require the game to be launched with the command line argument `scriptablehttp` (#162)
- Master server list will be used instead of the node system (load server list faster) (#234)

### Fixed

- Fixed issue with mouse acceleration when polling rate is greater than 125Hz (#230)
- Fixed issue with player speed caused by sprinting from the prone position (#232)
- Fixed client crash when cg_chatHeight was set to 0 (#237)
- Fixed GSC function Scr_TableLookupIStringByRow (#162)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.0] - 2022-05-01

### Added

- Added controller support (#75)
- Added aim assist for controllers (#75)
- Unlock camera_thirdPersonCrosshairOffset Dvar (#68)
- Added support for building custom Fonts with Zonebuilder (#88)
- Added colorblind friendly team colors (#101)
- Added emojis based on titlecards and emblems to use in the chat and server names Example: `:nuke:` (#130)
- Upon leaving a server 'archive' dvars (saved in the config file) will be reset to the value they had prior to joining the server (#134)
- Implement muteClient command for the game chat (#159)
- Implement unmute command for the game chat (#159)
- Add sv_allowAimAssist Dvar (#75)
- Add sv_allowColoredNames (#130)
- Add sv_randomMapRotation Dvar (#146)
- Add rcon_log_requests Dvar (#195)
- Add player_duckedSpeedScale Dvar (#141)
- Add player_proneSpeedScale Dvar (#141)
- Add cg_ufo_scaler Dvar (#158)
- Add cg_noclip_scaler Dvar (#158)
- Add bg_bouncesAllAngles Dvar (#158)
- Add bg_rocketJump Dvar (#158)
- Add bg_elevators Dvar (#156)
- Implement noclip client command (#152)
- Implement ufo client command (#152)
- Implement God client command (#152)
- Implement demigod client command (#152)
- Implement notarget client command (#152)
- Add noclip GSC Function (#152)
- Add ufo GSC Function (#152)
- Add God GSC Function (#152)
- Add demigod GSC Function (#152)
- Add notarget GSC Function (#152)
- Add replaceFunc GSC Function (#144)

### Changed

- Renamed sv_enableBounces to bg_bounces (#158)
- Renamed g_playerCollision to bg_playerEjection (#158)
- Renamed g_playerEjection to bg_playerCollision (#158)
- Setviewpos client command works outside private matches (#163)
- Ufo client command works outside of private matches (#152)
- Noclip client command works outside of private matches (#152)
- If a player name is less than 3 characters server will change it to `Unknown Soldier` (#130)
- scr_player_forceautoassign Dvar is false by default

### Fixed

- Fixed issue where CoD:O DLC Maps caused DirectX crash following `vid_restart` (#37)
- Fixes and improvements to Zonebuilder
- Fixed issue where the game froze following base game script throwing an error (#74)
- Fixed RCon on party servers (#91 - #95)
- Fixed slow motion during final killcams (#111 - #107)
- Fixed sound issue that causes the game to freeze (#106)
- Fixed issue where materials strings found in hostnames, player names, chat etc. caused the game to crash (#113)
- Fixed issue with servers displaying an invalid player count (#113)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.6.1] - 2020-12-23

### Added

- Add host information to /info endpoint (request)
- Add fileWrite GSC Function (#36)
- Add fileRead GSC Function (#36)
- Add fileExists GSC Function (#36)
- Add fileRemove GSC Function (#36)
- Add botMovement GSC Function (#46)
- Add botAction GSC Function (#46)
- Add botWeapon GSC Function (#46)
- Add botStop GSC Function (#46)
- Add isBot GSC Function (#46)
- Add setPing GSC Function (#46)
- Add GetSystemTime and GetSystemTimeMilliseconds GSC Functions (#46)
- Add PrintConsole GSC Function (#46)
- Add Exec GSC Function (#46)
- Add getIP GSC Method (#36)
- Add getPing GSC Method (#36)
- Add scr_intermissionTime GSC Function (#25)
- Add g_playerCollision Dvar (#36)
- Add g_playerEjection Dvar (#36)
- Add r_specularCustomMaps Dvar (#36)
- Unlock safeArea_horizontal and safeArea_vertical Dvars (#42)
- Unlock cg_fovscale Dvar (#47)
- Added g_antilag Dvar (#61)

### Changed

- Stats are now separate for each mod (#6). Player stats are copied to `fs_game` folder if no stats exist for this mod yet. Keep in mind this also means that level, XP and classes will not be synchronized with the main stats file after this point.
- Reduced duration of toasts (#48)
- Removed old updater functionality (#54)
- Use old bot names if bots.txt is not found (#46)

### Fixed

- Fixed a node system related crash (#45)
- Fixed an issue that made dedicated servers crash when info was requested during map rotation (#43)
- Fixed an issue where the game was trying to decrypt gsc files which caused it to crash when loading mods (#35)
- Fixed an issue causing the game to crash when Steam was running in the background (#56)
- Fixed slow download speed when using fast download

### Known issues

- HTTPS is not supported for fast downloads at the moment.

## [0.6.0] - 2018-12-30

### Added

- Implement unbanclient command.
- Implement /serverlist api endpoint on dedicated servers
- Add dvar to control the server query rate (net_serverQueryLimit & net_serverFrames)

### Changed

- Update dependencies
 
### Fixed

- Fix mods not working in private matches.
- Fix multiple game structures (map tools)
- Fix multiple vulnerability's
- Fix lag spikes on lower end PCs
- Fix invalid name check
- Fix openLink command crash issue
- Fix lobby server map downloading
- Fix steam integration

### Known issues

- HTTPS is not supported for fast downloads at the moment.

## [0.5.4] - 2017-07-09

### Added

- Integrate IW4MVM by luckyy.

### Changed

- Displayed stats for Dragunov have been changed, has no effect on actual game play.

### Fixed

- Fix fast download failing when the target host is missing a trailing slash.
- Fix servers not being listed in the server browser.
- Fix some FPS drop issues caused by compression code.

### Known issues

- HTTPS is not supported for fast downloads at the moment.

## [0.5.3] - 2017-07-02

### Added

- Increase webserver security.

### Fixed

- Reduce lags introduced by nodes.
- Fix modlist download.

## [0.5.2] - 2017-07-02

### Fixed

- Fix dedicated server crash.

## [0.5.1] - 2017-07-02

### Added

- Add fast download option for custom mods/maps based on Call of Duty 4.
- Display a toast when an update is available.
- Use the hourglass cursor while loading assets (with the native cursor feature).
- Show bots in parenthesis after the number of players in the serverlist (request).
- Add GSC event `level waittill("say", string, player);` (request).
- Restrict unauthorized mod download for password protected servers.
- Add OMA support for 15 classes.

### Changed

- Show friend avatars when they play IW4x (request).
- Rewrite and optimize the entire node system.
- Remove syncnode command for node system.
- Remove steam start.

### Fixed

- Fix lags and frame drops caused by server sorting.
- Fix demos on custom maps.
- Can no longer join a lobby or server with an incorrect password.
- Fix crashes caused by a bug in file/data compression.
- Improve overall stability.

## [0.5.0] - 2017-06-04

### Added

- Add GSC functionality to download files via HTTP(S) (request).
- Implement preliminary custom map support.

### Changed

- Add new nicknames for bots.
- Bumped Fastfile version. If you built fastfiles with the zone builder (e.g. mod.ff) you have to rebuild them!
- Default `sv_network_fps` to `1000` on dedicated game servers.
- Increase maximum FOV to 120.

### Fixed

- Fix `iw4x_onelog` dvar.
- Fix server list only showing local servers by default instead of Internet servers.
- Fix some deadlock situations on game shutdown.

## [0.4.2] - 2017-03-16

### Changed

- Disable unnecessary dvar update in server browser.
- Update bot names.

### Fixed

- Fix process permissions.
- Fix classic AK-47 color bleedthrough.
- Re-aligned the MG4's Raffica iron sights.

## [0.4.1] - 2017-03-10

### Fixed

- Fix command line argument parsing.

## [0.4.0] - 2017-03-10

### Added

- Set played with status.
- Add support for 15 classes.
- Add `iw4x_onelog` dvar.
- Add show server/playercount in server browser.

### Changed

- Do not show friend status notifications with stream friendly UI.
- Reduce loaded modules for dedis.
- Use joystick emblem for friend status.
- Disable XP bar when max level.
- Change fs_game display postition.
- Replace Painkiller with Juiced from IW5.

### Fixed

- Fix AK weaponfiles.
- Fix brightness slider.
- Fix text length for column mod in server browser.
- Changed the L86 LSW to use the correct HUD icon.
- Re-aligned the M93 Raffica's iron sights.
- Re-aligned the Desert Eagle's iron sights.

## [0.3.3] - 2017-02-14

### Added

- Add mapname to friend status (request).
- Add option to toggle notify friend state.
- Add support for mod.ff.

### Changed

- Disabled big minidump message box.
- Limit dedicated servers to 15 instances per IP.
- Move build number location.
- Remove news ticker and friends button from theater.

### Fixed

- Fix audio bug in options menu.
- Fix crash caused by faulty file download requests to game hosts.
- Fix friend sorting.
- Fix game not starting issue under certain circumstances.
- Fix menu crash.
- Fix typo in security increase popmenu.
- Fix vid_restart crash with connect protocol.
- Fix weapon crash issue.
- Potentially fix no-ammo bug.

## [0.3.2] - 2017-02-12

This is the third public Beta version.

### Added

- Add working friend system.
- Add colored pings in the server list.
- Add credits.
- Add first launch menu.
- Add AK-47 (Classic) attachments.
- Add HUD icon for night vision goggles.

### Changed

- Join when pressing enter in server list (request).
- Redesign and refactor all fullscreen menus.
- Increase weapon and configstring limit.
- Allow creating full crash dumps if wanted.
- Set default name from steam.

### Fixed

- Fix missing models on village.
- Fix custom server motd (request).
- Fix various memory leaks.
- Fix mouse pitch (request).
- Fix compatibility with B3 (request).
- Fix RCon bug (request).
- Fix dedicated server crash on linux.
- Fix crash in mod download.
- Fix peacekeeper reload sound.
- Fix cl_maxpackets 125 in settings (request).
- Fix deserteaglegold_mp icon.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.3.1] - 2017-01-21

This is the second public Beta version.

### Added

- Add classic AK-47 to CAC.
- Add servers to favorites when ingame.
- Add delete favorites button in the serverlist.

### Changed

- Change maplist to a dynamic list.

### Fixed

- Fix list focus.
- Fix mod restart loop.
- Fix mod download status.
- Fix modelsurf crash.
- Fix floating AK-74u.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.3.0] - 2017-01-14

This is the first public Beta version.

### Added

- Add com_logFilter dvar.
- Add peacekeeper.
- Add support for maps from DLC 8 (Recycled Pack)

  - Chemical Plant (mp_storm_spring)
  - Crash: Tropical (mp_crash_tropical)
  - Estate: Tropical (mp_estate_tropical)
  - Favela: Tropical (mp_fav_tropical)
  - Forgotten City (mp_bloc_sh)

### Changed

- Improve node synchronization handling.
- Improve security by modifying GUIDs to allow 64-bit certificate fingerprints.
- Optimize fastfiles, they are now a lot smaller.
- Replace intro.

### Fixed

- Fix concurrent image loading bug.
- Fix issues when spawning more than one bot.
- Fix no ammo bug.
- Fix server crash on startup.
- Fix splash screen hang.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.2.1] - 2016-12-14

This is the second public Alpha version.

### Added

- Add support for CoD:Online maps.

    - Firing Range (mp_firingrange)
    - Rust (mp_rust_long)
    - Shipment (mp_shipment/mp_shipment_long)

- Add `sv_motd` Dvar for server owners to set custom motd (will be visible in the loadscreen).
- Add Zonebuilder support for sounds and fx.
- Add command setviewpos.
- Add high-definition loadscreens.

### Changed

- Rename Arctic Wet Work map to it's official name (Freighter).
- Complete redesign of the main menus.
- Allow `cl_maxpackets` to be set up to 125.

### Fixed

- Fix crash when using the Harrier killstreak.
- Disable code that downloads news/changelog when in zonebuilder mode.
- Fix freeze on game shutdown.
- Disable unlockstats while ingame to prevent a crash.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.2.0] - 2016-10-09

This is the first public Alpha version.

### Added

- Support for CoD:Online maps.

    - Arctic Wet Work (mp_cargoship_sh)
    - Bloc (mp_bloc)
    - Bog (mp_bog_sh)
    - Crossfire (mp_cross_fire)
    - Killhouse (mp_killhouse)
    - Nuketown (mp_nuked)
    - Wet Work (mp_cargoship)

### Fixed

- Fix techniques in Zonebuilder.
- Fix possible memory leak.
- Fix timeout bug when connecting to server via iw4x link.
- Partially fix deadlock in decentralized networking code.

### Known issues

- Running IW4x on Linux currently requires gnutls to be installed additional to Wine as it needs to access the Tor service via HTTPS.

## [0.1.1] - 2016-09-19

This version is an internal Pre-Alpha version.

### Added

- Add IW5 material embedding system.

### Changed

- Enhance mod download with detailed progress display.

### Fixed

- Fix and optimize network logging commands.
- Fix console not displaying command inputs properly.
- Fix crash when running multiple instances of the IW4x client from the same directory.
- Fix crash when the `securityLevel` command is used on a game server.
- Fix possible data corruption in code for decentralized networking.
- Fix possible deadlock during client shutdown.

## [0.1.0] - 2016-09-03

This version is an internal Pre-Alpha version.

### Added

- Add `banclient` command which will permanently ban a client from a server. The ban will persist across restarts.
- Add capabilities to save played games and replay them ("Theater").
- Add code for generating and sending minidumps for debugging purposes. This feature is meant to be used only during the Open Beta and will be removed once the code goes to stable release.
- Add commands that allow forwarding console and games log via UDP to other computers ("network logging").
- Add D3D9Ex.
- Add filters for server list.
- Add handling for `iw4x://` URLs ("connect protocol"). For example, if IW4x is properly registered in Windows as URL handler for `iw4x://` URLs you can type `iw4x://ip:port`. If possible, this will connect to the server in an already running IW4x client.
- Add lean support through new key bindings.
- Add native cursor as replacement for the sometimes laggy in-game cursor. This change can be reverted in the settings menu.
- Add news ticker.
- Add remote console ("RCon").
- Add support for BigBrotherBot.
- Add support for hosting game mods in order to allow players to just join modded servers out of the box ("mod download").
- Add Warfare2 text coloring.
- Add zone builder. For more information see the respective documentation.
- Implement a completely decentralized peering network.
- Implement playlists which can be used for flexible map and gametype rotation.
- Introduce security levels. This ensures that you need to "pay" with CPU power to verify your identity once before being able to join a server which reduces the interval at which people who get banned can circumvent server bans through using new identities. The default security level is 23.
- Move IW4x resource files into their own folder to prevent clogging up the main game directories.
- Reintroduce parties, now also available for dedicated servers ("lobby servers").

### Changed

- Move logs to `userraw` folder.
- Replace main menu background music.
