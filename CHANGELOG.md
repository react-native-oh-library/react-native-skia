# Changelog
## 鸿蒙化Log

### v2.0.1-rc.1

- fix:修复使用RN框架的release包运行导致crash问题

### v2.0.0-rc.2

- fix:修复release模式编译失败问题以及运行crash问题

### v2.0.0-rc.1

- feat:react-native-skia升级2.4.14版本适配0.82框架

### v1.4.0-rc.1

- pre-release version 1.4.0-rc.1

### v1.3.8-rc.1

- pre-release version 1.3.8-rc.1


* feat: add OpenHarmony support for react-native-skia

## ReleaseLog
### v1.12.4

#### Bug Fixes

- remove potential buffer over-reads ([#3086](https://github.com/Shopify/react-native-skia/issues/3086)) ([afeab61](https://github.com/Shopify/react-native-skia/commit/afeab61bf1693979af584d53fce78712c55beeab))
- always seperate type imports from normal imports ([#3070](https://github.com/Shopify/react-native-skia/issues/3070)) ([09aa642](https://github.com/Shopify/react-native-skia/commit/09aa642de4aefcc5c5c75a59020700fe96f925ab))
- BoxShadow color with opacity no longer get ignored ([#3093](https://github.com/Shopify/react-native-skia/issues/3093)) ([66d44b1](https://github.com/Shopify/react-native-skia/commit/66d44b18b2e5f5a4d2a02c60399dd2bfefa2c3e9))

### v2.0.0-next.2

#### Bug Fixes

- **🌎:** Consolidate Enum access on RN Web ([#3063](https://github.com/Shopify/react-native-skia/issues/3063)) ([5ecba93](https://github.com/Shopify/react-native-skia/commit/5ecba9348969c46011f022791eaa4069e2976f2d))
- **🌎:** fix cloneInstance bug on RN Web ([#2998](https://github.com/Shopify/react-native-skia/issues/2998)) ([dc3b5f3](https://github.com/Shopify/react-native-skia/commit/dc3b5f3206c784b2b727744d715594240eba21b2))
- **🌎:** Fix first frame crash on RN Web ([#3017](https://github.com/Shopify/react-native-skia/issues/3017)) ([38476b5](https://github.com/Shopify/react-native-skia/commit/38476b5f4b6d7dd46e0d9fabe5b9346a617a0554))
- **🌎:** reconciller bug on React Native Web ([#3050](https://github.com/Shopify/react-native-skia/issues/3050)) ([006bc63](https://github.com/Shopify/react-native-skia/commit/006bc639fce5d73f3457e227e5a30aa975cfeea4))
- **🌎:** Upgrade to CanvasKit 0.40 ([#3073](https://github.com/Shopify/react-native-skia/issues/3073)) ([f299931](https://github.com/Shopify/react-native-skia/commit/f29993169d187bc9114313fd4694c8cbf1d28da7))
- **🍏:** Fix release bug on apple platforms ([#3067](https://github.com/Shopify/react-native-skia/issues/3067)) ([89148d1](https://github.com/Shopify/react-native-skia/commit/89148d1fdd12873497492abc9c7060ff086a254c)), closes [#3056](https://github.com/Shopify/react-native-skia/issues/3056)
- **🏎️:** minor improvement to the first time to frame in reconciler ([#2991](https://github.com/Shopify/react-native-skia/issues/2991)) ([0d4e4e4](https://github.com/Shopify/react-native-skia/commit/0d4e4e4a5d5f120091b29106ce00c277ccda7c26))
- **🏗️:** set ZERO_AR_DATE in env of build command ([#3031](https://github.com/Shopify/react-native-skia/issues/3031)) ([7c92aba](https://github.com/Shopify/react-native-skia/commit/7c92aba91c25c09a1ca7a11e34fcf60228621c8a))
- **🐎:** fix serious threading issue in useAnimatedImageValue ([#3057](https://github.com/Shopify/react-native-skia/issues/3057)) ([d366abd](https://github.com/Shopify/react-native-skia/commit/d366abd8a00a51df5f53aa7a2854b6b1c2519354))
- **🐛:** fix bug in release process ([#3019](https://github.com/Shopify/react-native-skia/issues/3019)) ([ff7bf1c](https://github.com/Shopify/react-native-skia/commit/ff7bf1c00d5694ddf9ad1c4a27be0228374485c9))
- **💚:** add support for testID on Canvas in Jest ([#3058](https://github.com/Shopify/react-native-skia/issues/3058)) ([2d0c342](https://github.com/Shopify/react-native-skia/commit/2d0c3420aa136b8f930d2c50ca4981484ef914ae))
- **💚:** fix release script ([#3024](https://github.com/Shopify/react-native-skia/issues/3024)) ([c8929f5](https://github.com/Shopify/react-native-skia/commit/c8929f5b609d2956b7eaf8e694648a43da4b7e7e))
- **💚:** fix release script ([#3025](https://github.com/Shopify/react-native-skia/issues/3025)) ([9fb295b](https://github.com/Shopify/react-native-skia/commit/9fb295b3fe26cd1edb9e96381720a7b9a40340f3))
- **💚:** fix release script ([#3026](https://github.com/Shopify/react-native-skia/issues/3026)) ([ed851c2](https://github.com/Shopify/react-native-skia/commit/ed851c298e0253fdac95212365721e0df11ded44))
- **💚:** release script ([#3028](https://github.com/Shopify/react-native-skia/issues/3028)) ([d21f0eb](https://github.com/Shopify/react-native-skia/commit/d21f0eb2c237c3213205e7bf810f14566f6b2a44))
- **📦:** fix release script ([#3023](https://github.com/Shopify/react-native-skia/issues/3023)) ([eb6a8be](https://github.com/Shopify/react-native-skia/commit/eb6a8beecb048b95f49d72c6efe283401e2af7fe))
- **🤖:** Allow customizing ndk path and version ([#3018](https://github.com/Shopify/react-native-skia/issues/3018)) ([3b938b1](https://github.com/Shopify/react-native-skia/commit/3b938b16d3f294ba9f41a34a7c8ae1ac768ac966))
- **🤖:** fix minor memory leak on Android ([#3052](https://github.com/Shopify/react-native-skia/issues/3052)) ([1fcdf51](https://github.com/Shopify/react-native-skia/commit/1fcdf516c9340b2e771dc1c4484dba5e89947af4))
- **🤖:** Revert minSDK=26 for now ([#3075](https://github.com/Shopify/react-native-skia/issues/3075)) ([4a41189](https://github.com/Shopify/react-native-skia/commit/4a41189bd50d8d49eaa4ffaa6d5eed0ce666fe9f))
- **🪢:** Fix color type in headless mode ([#3071](https://github.com/Shopify/react-native-skia/issues/3071)) ([e71451e](https://github.com/Shopify/react-native-skia/commit/e71451eb523f950a98b3b3248b0edb30b58b87f2))
- consistent std::move semantics ([#3040](https://github.com/Shopify/react-native-skia/issues/3040)) ([77c2a4f](https://github.com/Shopify/react-native-skia/commit/77c2a4f6b262092809b2f51bd080ae968658b255))
- harden shaders against buffer over-reads ([#3066](https://github.com/Shopify/react-native-skia/issues/3066)) ([6bdfa74](https://github.com/Shopify/react-native-skia/commit/6bdfa7401ee7695a4c67a3e1890dd30e323c303e))
- harden vertices ([#3065](https://github.com/Shopify/react-native-skia/issues/3065)) ([67432c2](https://github.com/Shopify/react-native-skia/commit/67432c20ba64ec27187e0e0b6fa42a26459786c9))
- incorrect error message on `getPosTan` ([#3041](https://github.com/Shopify/react-native-skia/issues/3041)) ([807b63e](https://github.com/Shopify/react-native-skia/commit/807b63e757885072cb44e43f7725d79b12145374))
- throw error when accessing disposed objects ([#3043](https://github.com/Shopify/react-native-skia/issues/3043)) ([b7743d8](https://github.com/Shopify/react-native-skia/commit/b7743d89c9a5eec607636455572205f1309a9ff3))
- use `JsiSkPathEffect` on dispose export ([#3055](https://github.com/Shopify/react-native-skia/issues/3055)) ([74624c0](https://github.com/Shopify/react-native-skia/commit/74624c05c4f57f1177f24f5825166dd966c2dadf))

#### Features

- **💚:** macOS support ([#3045](https://github.com/Shopify/react-native-skia/issues/3045)) ([e029398](https://github.com/Shopify/react-native-skia/commit/e029398b2c67e364da2f7aa9d0acd908d0c0f393))

### v1.12.3

#### Bug Fixes

- **🌎:** Upgrade to CanvasKit 0.40 ([#3073](https://github.com/Shopify/react-native-skia/issues/3073)) ([f299931](https://github.com/Shopify/react-native-skia/commit/f29993169d187bc9114313fd4694c8cbf1d28da7))
- **🪢:** Fix color type in headless mode ([#3071](https://github.com/Shopify/react-native-skia/issues/3071)) ([e71451e](https://github.com/Shopify/react-native-skia/commit/e71451eb523f950a98b3b3248b0edb30b58b87f2))
- harden shaders against buffer over-reads ([#3066](https://github.com/Shopify/react-native-skia/issues/3066)) ([6bdfa74](https://github.com/Shopify/react-native-skia/commit/6bdfa7401ee7695a4c67a3e1890dd30e323c303e))
- harden vertices ([#3065](https://github.com/Shopify/react-native-skia/issues/3065)) ([67432c2](https://github.com/Shopify/react-native-skia/commit/67432c20ba64ec27187e0e0b6fa42a26459786c9))

### v1.12.2

#### Bug Fixes

- **🌎:** Consolidate Enum access on RN Web ([#3063](https://github.com/Shopify/react-native-skia/issues/3063)) ([5ecba93](https://github.com/Shopify/react-native-skia/commit/5ecba9348969c46011f022791eaa4069e2976f2d))
- **🍏:** Fix release bug on apple platforms ([#3067](https://github.com/Shopify/react-native-skia/issues/3067)) ([89148d1](https://github.com/Shopify/react-native-skia/commit/89148d1fdd12873497492abc9c7060ff086a254c)), closes [#3056](https://github.com/Shopify/react-native-skia/issues/3056)

### v1.12.1

#### Bug Fixes

- **🐎:** fix serious threading issue in useAnimatedImageValue ([#3057](https://github.com/Shopify/react-native-skia/issues/3057)) ([d366abd](https://github.com/Shopify/react-native-skia/commit/d366abd8a00a51df5f53aa7a2854b6b1c2519354))
- **💚:** add support for testID on Canvas in Jest ([#3058](https://github.com/Shopify/react-native-skia/issues/3058)) ([2d0c342](https://github.com/Shopify/react-native-skia/commit/2d0c3420aa136b8f930d2c50ca4981484ef914ae))
- **🤖:** fix minor memory leak on Android ([#3052](https://github.com/Shopify/react-native-skia/issues/3052)) ([1fcdf51](https://github.com/Shopify/react-native-skia/commit/1fcdf516c9340b2e771dc1c4484dba5e89947af4))
- consistent std::move semantics ([#3040](https://github.com/Shopify/react-native-skia/issues/3040)) ([77c2a4f](https://github.com/Shopify/react-native-skia/commit/77c2a4f6b262092809b2f51bd080ae968658b255))
- incorrect error message on `getPosTan` ([#3041](https://github.com/Shopify/react-native-skia/issues/3041)) ([807b63e](https://github.com/Shopify/react-native-skia/commit/807b63e757885072cb44e43f7725d79b12145374))
- throw error when accessing disposed objects ([#3043](https://github.com/Shopify/react-native-skia/issues/3043)) ([b7743d8](https://github.com/Shopify/react-native-skia/commit/b7743d89c9a5eec607636455572205f1309a9ff3))
- use `JsiSkPathEffect` on dispose export ([#3055](https://github.com/Shopify/react-native-skia/issues/3055)) ([74624c0](https://github.com/Shopify/react-native-skia/commit/74624c05c4f57f1177f24f5825166dd966c2dadf))

### v1.12.0

#### Bug Fixes

- **🌎:** reconciller bug on React Native Web ([#3050](https://github.com/Shopify/react-native-skia/issues/3050)) ([006bc63](https://github.com/Shopify/react-native-skia/commit/006bc639fce5d73f3457e227e5a30aa975cfeea4))
- **🏗️:** set ZERO_AR_DATE in env of build command ([#3031](https://github.com/Shopify/react-native-skia/issues/3031)) ([7c92aba](https://github.com/Shopify/react-native-skia/commit/7c92aba91c25c09a1ca7a11e34fcf60228621c8a))

#### Features

- **💚:** macOS support ([#3045](https://github.com/Shopify/react-native-skia/issues/3045)) ([e029398](https://github.com/Shopify/react-native-skia/commit/e029398b2c67e364da2f7aa9d0acd908d0c0f393))

### v1.11.18

#### Bug Fixes

- **💚:** release script ([#3028](https://github.com/Shopify/react-native-skia/issues/3028)) ([d21f0eb](https://github.com/Shopify/react-native-skia/commit/d21f0eb2c237c3213205e7bf810f14566f6b2a44))

### v1.11.12

#### Bug Fixes

- **🌎:** Fix first frame crash on RN Web ([#3017](https://github.com/Shopify/react-native-skia/issues/3017)) ([38476b5](https://github.com/Shopify/react-native-skia/commit/38476b5f4b6d7dd46e0d9fabe5b9346a617a0554))

### v2.0.0-next.1

#### Bug Fixes

- **💚:** revert bogus 1.11.9 release ([#2987](https://github.com/Shopify/react-native-skia/issues/2987)) ([673e21e](https://github.com/Shopify/react-native-skia/commit/673e21edbcc09fa615931cd5a31567c37d4501ae))

#### Features

- React 19/ RN 0.78 upgrade ([#2993](https://github.com/Shopify/react-native-skia/issues/2993)) ([b9a2345](https://github.com/Shopify/react-native-skia/commit/b9a2345667da47eb8bd379282fa21d6c4a2d8cf3))

#### BREAKING CHANGES

- - React 19 and React Native 0.78 minimum are required
- SDK 26 minimum is required on Android
- The `debug` and `mode` properties are now deprecated
- `SKSGRoot` api is now async, as a result functions such as `drawAsImage` are now async as well.

### v1.11.11

#### Bug Fixes

- **🌎:** fix cloneInstance bug on RN Web ([#2998](https://github.com/Shopify/react-native-skia/issues/2998)) ([dc3b5f3](https://github.com/Shopify/react-native-skia/commit/dc3b5f3206c784b2b727744d715594240eba21b2))

### v1.11.10

#### Bug Fixes

- **🏎️:** minor improvement to the first time to frame in reconciler ([#2991](https://github.com/Shopify/react-native-skia/issues/2991)) ([0d4e4e4](https://github.com/Shopify/react-native-skia/commit/0d4e4e4a5d5f120091b29106ce00c277ccda7c26))
- **💚:** revert bogus 1.11.9 release ([#2987](https://github.com/Shopify/react-native-skia/issues/2987)) ([673e21e](https://github.com/Shopify/react-native-skia/commit/673e21edbcc09fa615931cd5a31567c37d4501ae))

### v1.11.9

#### Bug Fixes

- **⬆️:** Upgrade to Skia m134 ([#2978](https://github.com/Shopify/react-native-skia/issues/2978)) ([29109f1](https://github.com/Shopify/react-native-skia/commit/29109f19aaac57f08c96e881ff3ab55b7d82aa5e))

#### Reverts

- Revert "Upgrade to React 19 ([#2984](https://github.com/Shopify/react-native-skia/issues/2984))" ([#2985](https://github.com/Shopify/react-native-skia/issues/2985)) ([7da94f6](https://github.com/Shopify/react-native-skia/commit/7da94f67c7e466b7daca99781f2401e4a3646860))

### v1.11.9-next.1

#### Bug Fixes

- **⬆️:** Upgrade to Skia m134 ([#2978](https://github.com/Shopify/react-native-skia/issues/2978)) ([29109f1](https://github.com/Shopify/react-native-skia/commit/29109f19aaac57f08c96e881ff3ab55b7d82aa5e))

###  v1.11.8

#### Bug Fixes

- **🌍:** minor issue on RN Web ([#2963](https://github.com/Shopify/react-native-skia/issues/2963)) ([6443ac2](https://github.com/Shopify/react-native-skia/commit/6443ac2ee3045411bfe48ae143d58771a9f082a4))

###  v1.11.7

#### Bug Fixes

- **🌎:** Fix Web support bug ([#2958](https://github.com/Shopify/react-native-skia/issues/2958)) ([f33ae0f](https://github.com/Shopify/react-native-skia/commit/f33ae0f8752ce73e3bd27452dc6111dd3eaba043))

###  v1.11.6

#### Bug Fixes

- **🌍:** Fix RN Web support ([#2954](https://github.com/Shopify/react-native-skia/issues/2954)) ([121ebd4](https://github.com/Shopify/react-native-skia/commit/121ebd49a518aab0c0cb8885ecfdcf411a8d4fc8))
- **📝:** Fix dangling pointer in ([#2953](https://github.com/Shopify/react-native-skia/issues/2953)) ([55de322](https://github.com/Shopify/react-native-skia/commit/55de322df718a1ddf0651befcf3fc6269c2cc30d))

###  v1.11.5

#### Bug Fixes

- **🌎:** fix issue where users were running into a "React is not defined" error using Skia on web for both Next.js and Storybook. ([#2941](https://github.com/Shopify/react-native-skia/issues/2941)) ([c6d81c4](https://github.com/Shopify/react-native-skia/commit/c6d81c47ec8a27805af1965cb781b7d8dc20c2eb))
- **🌧️:** fix major regression with the Glyphs component ([#2944](https://github.com/Shopify/react-native-skia/issues/2944)) ([53e8fae](https://github.com/Shopify/react-native-skia/commit/53e8fae35ea87945dda4c115a1068c0b65b0e5fd))
- **🐯:** fix svg regression in the new reconciller ([#2942](https://github.com/Shopify/react-native-skia/issues/2942)) ([4bc9b79](https://github.com/Shopify/react-native-skia/commit/4bc9b7979ff595e658c189f4b5e7e905c812c308))

###  v1.11.4

#### Bug Fixes

- **🎮:** fix headless mode ([#2937](https://github.com/Shopify/react-native-skia/issues/2937)) ([34d56fb](https://github.com/Shopify/react-native-skia/commit/34d56fbf563926a2765aab6dd6e4a19dcab68b5f))
- **🔗:** remove deadlink in the documentation ([#2939](https://github.com/Shopify/react-native-skia/issues/2939)) ([49be75d](https://github.com/Shopify/react-native-skia/commit/49be75dc7006a38a122bc797e59cf3ba0dc9fb96))
- **🖼️:** remove uncessary JSI calls ([#2934](https://github.com/Shopify/react-native-skia/issues/2934)) ([7934c6f](https://github.com/Shopify/react-native-skia/commit/7934c6fd504750e512bf3ff4496a4bc434ba44ec))

###  v1.11.3

#### Bug Fixes

- **🍏:** Fix iOS linking error on RN 0.77 ([#2933](https://github.com/Shopify/react-native-skia/issues/2933)) ([8be7039](https://github.com/Shopify/react-native-skia/commit/8be70390277c8690eed1c515142555a32d821d83)), closes [#2931](https://github.com/Shopify/react-native-skia/issues/2931)

###  v1.11.2

#### Bug Fixes

- **⚛️:** react-native 0.77 build on Android ([#2926](https://github.com/Shopify/react-native-skia/issues/2926)) ([a69d9bc](https://github.com/Shopify/react-native-skia/commit/a69d9bcc1234ab01d3cd1aa9154941360fd6d7c0))
- **⚛️:** react-native 0.77 build on Android ([#2926](https://github.com/Shopify/react-native-skia/issues/2926)) ([75f713c](https://github.com/Shopify/react-native-skia/commit/75f713c2090cfe524b1c0b35ede5f7e26200f498))
- **🐛:** fix bug where unmounted canvases would be drawn ([#2929](https://github.com/Shopify/react-native-skia/issues/2929)) ([815051b](https://github.com/Shopify/react-native-skia/commit/815051be5a5fe84efcd21e4943295619ddbcd5a5))
- **🤖:** fix minor issue with the opaque property on Android ([0e5cd91](https://github.com/Shopify/react-native-skia/commit/0e5cd910c2bb643c8a88a59f710cb17d620837e2))

### v1.11.2

#### Bug Fixes

- **🐛:** fix conflicting header name on iOS ([#2924](https://github.com/Shopify/react-native-skia/issues/2924)) ([c026636](https://github.com/Shopify/react-native-skia/commit/c026636455dbb376f9e7f9e88b16fbe7cfe4a80b))

### v1.11.0

#### Bug Fixes

- **📕:** added font mock to skia jest setup ([#2903](https://github.com/Shopify/react-native-skia/issues/2903)) ([d029b60](https://github.com/Shopify/react-native-skia/commit/d029b6048e5b4acd10ba4c9fd12f247bcf28dfc7))

#### Features

- **🏎️:** Substancial performance improvements in the reconciler ([#2919](https://github.com/Shopify/react-native-skia/issues/2919)) ([acd63ce](https://github.com/Shopify/react-native-skia/commit/acd63ceb0b11a85430be3b0d0d1c30d54d3427a7))

### v1.10.2

#### Bug Fixes

- **🤖:** remove unstable React Native Screens workaround on Android ([#2894](https://github.com/Shopify/react-native-skia/issues/2894)) ([02d3e13](https://github.com/Shopify/react-native-skia/commit/02d3e1343f9b3d9f2baf6218e0a8f6b10b6fefc8))

### v1.10.1

#### Bug Fixes

- **🎨:** consolidate color processing in the scenegraph ([#2891](https://github.com/Shopify/react-native-skia/issues/2891)) ([939ad0b](https://github.com/Shopify/react-native-skia/commit/939ad0b7300aaa8f6cfe5f92189e62d95647dc8b))

### v1.10.0

#### Features

- **🎨:** Enable new reconciler by default on iOS and Android ([#2865](https://github.com/Shopify/react-native-skia/issues/2865)) ([6bc0508](https://github.com/Shopify/react-native-skia/commit/6bc050872d67e473da6716b3d99647ab51d09628))

### v1.9.1

#### Bug Fixes

- **🌎:** fix bug with sampling options in drawAtlas ([#2886](https://github.com/Shopify/react-native-skia/issues/2886)) ([ec9c8c9](https://github.com/Shopify/react-native-skia/commit/ec9c8c902023ba3c727d978cbb9d6218c57d1c7a))

### v1.9.0

#### Bug Fixes

- **🐎:** minor improvements to the new reconciler ([#2871](https://github.com/Shopify/react-native-skia/issues/2871)) ([c5f48cc](https://github.com/Shopify/react-native-skia/commit/c5f48cc4278bd32c3b8bd6eb99257473c47d2477))

#### Features

- **🍏:** add tvOS support ([#2857](https://github.com/Shopify/react-native-skia/issues/2857)) ([16abb40](https://github.com/Shopify/react-native-skia/commit/16abb40e90bba9e8d6c334667560ce414da4557b))
- **🏞️:** fix default image sampling and add full control of sampling options ([#2880](https://github.com/Shopify/react-native-skia/issues/2880)) ([8b14242](https://github.com/Shopify/react-native-skia/commit/8b142425658ded2ede8b5440606beac56bc02e74)), closes [#2507](https://github.com/Shopify/react-native-skia/issues/2507)

### v1.8.2

#### Bug Fixes

- **🐛:** Fix serious memory error with data loading hooks (useImage, useFont) ([#2866](https://github.com/Shopify/react-native-skia/issues/2866)) ([a78b255](https://github.com/Shopify/react-native-skia/commit/a78b255fb2e10284ec15aef86ad32aaa434cdc9c))

### v1.8.1

#### Bug Fixes

- **🐎:** remove reanimated 2 support ([#2864](https://github.com/Shopify/react-native-skia/issues/2864)) ([167e4e2](https://github.com/Shopify/react-native-skia/commit/167e4e24f157814e8d0a06b1e2f2dd1c1cb65e1f))
- **🖼️:** improved scene graph model (RN Web only for now) ([#2863](https://github.com/Shopify/react-native-skia/issues/2863)) ([fef5aba](https://github.com/Shopify/react-native-skia/commit/fef5aba79ab9904862528110a0864b2cfdc686d0))

### v1.8.0

#### Features

- **🌎:** Enable new reconciler on RN Web ([#2854](https://github.com/Shopify/react-native-skia/issues/2854)) ([393dd76](https://github.com/Shopify/react-native-skia/commit/393dd76bbda3813f81da0beae634db0b37c5297b))

### v1.7.7

#### Bug Fixes

- **🐎:** minor improvement to the reanimated package detection ([#2853](https://github.com/Shopify/react-native-skia/issues/2853)) ([defff6c](https://github.com/Shopify/react-native-skia/commit/defff6c7ce5da029cd0b5844a8f4372ea9339788))
- **🤖:** fix resizing on Android ([#2840](https://github.com/Shopify/react-native-skia/issues/2840)) ([9d59c74](https://github.com/Shopify/react-native-skia/commit/9d59c745641e65b2d3bf6d94ecbb62e5d1eb1357)), closes [#2831](https://github.com/Shopify/react-native-skia/issues/2831)
- **🚤:** fix bug on first frame ([#2842](https://github.com/Shopify/react-native-skia/issues/2842)) ([ccf403c](https://github.com/Shopify/react-native-skia/commit/ccf403c8d6bc84a8dd6dab7f96f5e884cb40731c))

### v1.7.6

#### Bug Fixes

- **⏺️:** fix bug in isRRect ([#2822](https://github.com/Shopify/react-native-skia/issues/2822)) ([b7a55c5](https://github.com/Shopify/react-native-skia/commit/b7a55c5213d3a6bd31c196d26bbd0a8c57060047))
- **🎨:** fix race condition in SkiaDomView ([#2823](https://github.com/Shopify/react-native-skia/issues/2823)) ([cca99c4](https://github.com/Shopify/react-native-skia/commit/cca99c4782855781d9c0e0294f820e354bda2ea6)), closes [#2814](https://github.com/Shopify/react-native-skia/issues/2814)
- **📝:** fix bug with textAlign property in paragraph style ([#2821](https://github.com/Shopify/react-native-skia/issues/2821)) ([bce4183](https://github.com/Shopify/react-native-skia/commit/bce41836e49b7fa2a76f7dc2287f9219746c786e))
- **🖼️:** fix minor bug with LinearGammaColorFilter node factory ([#2820](https://github.com/Shopify/react-native-skia/issues/2820)) ([e77754c](https://github.com/Shopify/react-native-skia/commit/e77754c9aad79998a0df830d0333c25d965cede6))

### v1.7.5

#### Bug Fixes

- **⏺️:** fix memory error with rounded rectangles ([#2812](https://github.com/Shopify/react-native-skia/issues/2812)) ([b72c441](https://github.com/Shopify/react-native-skia/commit/b72c441debcb97ba9549526a2217e99cde21877b))
- **🌎:** Add makeImageSnapshotAsync() on Web ([#2810](https://github.com/Shopify/react-native-skia/issues/2810)) ([7511466](https://github.com/Shopify/react-native-skia/commit/751146684aeaf37ffe3f6e61b44c938838b89d4a))
- **🎨:** fix bug in drawAtlas() ([#2809](https://github.com/Shopify/react-native-skia/issues/2809)) ([88980ba](https://github.com/Shopify/react-native-skia/commit/88980ba4e96c0c33d03b390e38281460ff163a1c))
- **🐯:** fix minor parameter bug in drawSvg() ([#2811](https://github.com/Shopify/react-native-skia/issues/2811)) ([bd526a5](https://github.com/Shopify/react-native-skia/commit/bd526a5e6c30a9739e1a81f878a2249d4910d107))
- **🤖:** Fix regression with PictureView on Android ([#2813](https://github.com/Shopify/react-native-skia/issues/2813)) ([84a3b3a](https://github.com/Shopify/react-native-skia/commit/84a3b3aead96e2abec2e722ae678656683befa5d))

### v1.7.4

#### Bug Fixes

- **🤖:** Regression with java exceptions on Android ([#2808](https://github.com/Shopify/react-native-skia/issues/2808)) ([9ce6660](https://github.com/Shopify/react-native-skia/commit/9ce6660f2a455e9f3604a20f243e0ee3e1aba191))

### v1.7.3

#### Bug Fixes

- **🤖:** fix build on Android ([#2806](https://github.com/Shopify/react-native-skia/issues/2806)) ([1908603](https://github.com/Shopify/react-native-skia/commit/1908603fd63111d95d893cae3df2530679162155))

### v1.7.2

#### Bug Fixes

- **🤖:** fix vulkan/angle support ([#2801](https://github.com/Shopify/react-native-skia/issues/2801)) ([e9bb166](https://github.com/Shopify/react-native-skia/commit/e9bb166fe793679ff7988b8cc75cdc75dc364295))

### v1.7.1

#### Bug Fixes

- **🍏:** use dedicated bridge ([#2760](https://github.com/Shopify/react-native-skia/issues/2760)) ([2cfa890](https://github.com/Shopify/react-native-skia/commit/2cfa890a86224e326306e6c2e562fcda6e7f5614))

### v1.7.0

#### Bug Fixes

- **🍏:** fix reparenting of View on iOS ([#2793](https://github.com/Shopify/react-native-skia/issues/2793)) ([de87571](https://github.com/Shopify/react-native-skia/commit/de875718291d26867f0aee3f69f59886e5f1d823)), closes [#2732](https://github.com/Shopify/react-native-skia/issues/2732) [#2636](https://github.com/Shopify/react-native-skia/issues/2636)

#### Features

- **🏞️:** cross context texture sharing and external texture import ([#2782](https://github.com/Shopify/react-native-skia/issues/2782)) ([f9c8119](https://github.com/Shopify/react-native-skia/commit/f9c81193a40feb0bc3107f564d189c46831f3c39))
- **🧮:** Add invert on Matrix4 ([#2791](https://github.com/Shopify/react-native-skia/issues/2791)) ([42f81f0](https://github.com/Shopify/react-native-skia/commit/42f81f05a66c6e083eb232ad9cc55b8e72405413))

### v1.6.0

#### Bug Fixes

- **🖍️:** fix regression in ColorType enum ([#2779](https://github.com/Shopify/react-native-skia/issues/2779)) ([06ecae8](https://github.com/Shopify/react-native-skia/commit/06ecae8e895c53b50de798d35563e32abc148aaa))
- **🤖:** fix bug with default clear color on Android offscreen surfaces ([#2781](https://github.com/Shopify/react-native-skia/issues/2781)) ([3401111](https://github.com/Shopify/react-native-skia/commit/3401111d672eaed516a78dcd2eb2aae36427bf76))

#### Features

- **👨🏻‍🎨:** Opaque property ([#2776](https://github.com/Shopify/react-native-skia/issues/2776)) ([88dec7f](https://github.com/Shopify/react-native-skia/commit/88dec7f6f9bdbcf1374e96e76f59af4a354b4b24))

### v1.5.10

#### Bug Fixes

- **🐛:** fix build regression on fabric ([#2769](https://github.com/Shopify/react-native-skia/issues/2769)) ([aab5771](https://github.com/Shopify/react-native-skia/commit/aab5771a80842d26ad721719a268691847c56ba1))
- **🐛:** Fix dangling pointer in requestRedraw ([#2768](https://github.com/Shopify/react-native-skia/issues/2768)) ([412d3a3](https://github.com/Shopify/react-native-skia/commit/412d3a35b2e10bba24fa27967321a94f7d9e387e))

### v1.5.9

#### Bug Fixes

- **🤖:** Remove internal draw loop ([#2763](https://github.com/Shopify/react-native-skia/issues/2763)) ([ba1db84](https://github.com/Shopify/react-native-skia/commit/ba1db84d5f5403831aa3a59cbe2fcdf118ceac74))

### v1.5.8

#### Bug Fixes

- **🖼️:** fix thread safety in SkImage and viewRef.makeImageSnapshot ([#2761](https://github.com/Shopify/react-native-skia/issues/2761)) ([f4db5f5](https://github.com/Shopify/react-native-skia/commit/f4db5f540f8535cdd3b9d9e75dc0446eba1643bc))

### v1.5.7

#### Bug Fixes

- **🤖:** bug fixes in OpenGL renderer ([#2759](https://github.com/Shopify/react-native-skia/issues/2759)) ([d314745](https://github.com/Shopify/react-native-skia/commit/d3147450a77a112c544d4bb6478b13504f332044))

### v1.5.6

#### Bug Fixes

- **💚:** Fix iOS segfault error for surfaces with 0 size ([#2753](https://github.com/Shopify/react-native-skia/issues/2753)) ([ad6e7d0](https://github.com/Shopify/react-native-skia/commit/ad6e7d0c83d43a13d00b7dc4de08820261e73fdd))

### v1.5.5

#### Bug Fixes

- **🤖:** Fix serious Android threading issue ([#2749](https://github.com/Shopify/react-native-skia/issues/2749)) ([6fbaeab](https://github.com/Shopify/react-native-skia/commit/6fbaeab7621862e11adf504894a7ffd52220533f))

### v1.5.4

#### Bug Fixes

- **🍏:** Fix performance regression on iOS ([#2744](https://github.com/Shopify/react-native-skia/issues/2744)) ([53a8db5](https://github.com/Shopify/react-native-skia/commit/53a8db5216889936a5504134a2bc3a91894cf0b4)), closes [#2743](https://github.com/Shopify/react-native-skia/issues/2743)
- **🪨:** enable experiment Graphite via build flag ([#2736](https://github.com/Shopify/react-native-skia/issues/2736)) ([5c0f38f](https://github.com/Shopify/react-native-skia/commit/5c0f38fa25edafa0d876d5a5f9a1a927ed4a5ad8))

### v1.5.3

#### Bug Fixes

- **💚:** fix regression with pixel density on iOS ([#2726](https://github.com/Shopify/react-native-skia/issues/2726)) ([d10a413](https://github.com/Shopify/react-native-skia/commit/d10a413a2ff34eb6f954c7a4ed374bada0e5f6bb))

### v1.5.2

#### Bug Fixes

- **🤖:** fix minor threading issue on Android ([#2725](https://github.com/Shopify/react-native-skia/issues/2725)) ([569acc9](https://github.com/Shopify/react-native-skia/commit/569acc9c2d670659b84509b928b0f3192f6060cf))

### v1.5.1

#### Bug Fixes

- **🍏:** support for ProMotion 120hz ([#2690](https://github.com/Shopify/react-native-skia/issues/2690)) ([5095c44](https://github.com/Shopify/react-native-skia/commit/5095c445065af05844f7bad01698f88da071c18c))

### v1.5.0

#### Bug Fixes

- **⬆️:** Upgraded Android build to support RN 0.76 ([#2694](https://github.com/Shopify/react-native-skia/issues/2694)) ([569d071](https://github.com/Shopify/react-native-skia/commit/569d071f3187b436fc1b2bdb38c74b73d962bfc8))

#### Features

- **🔥:** Remove deprecated onTouch prop ([#2669](https://github.com/Shopify/react-native-skia/issues/2669)) ([d2b5e8b](https://github.com/Shopify/react-native-skia/commit/d2b5e8b30d8d2d51b9f18c94bf57d2bdd39111b2))

### v1.4.2

#### Bug Fixes

- **💚:** add missing xcframeworks files and remove duplicated headers ([#2671](https://github.com/Shopify/react-native-skia/issues/2671)) ([f4db230](https://github.com/Shopify/react-native-skia/commit/f4db230562b2b8d699c0cfe02086b96eb67ce8a2))
- **🤖:** typo in directive ([#2668](https://github.com/Shopify/react-native-skia/issues/2668)) ([bf416d8](https://github.com/Shopify/react-native-skia/commit/bf416d8d3c0d7250be2693533ca06e8b9c5e13f4)), closes [#2661](https://github.com/Shopify/react-native-skia/issues/2661)

### v1.4.1

#### Bug Fixes

- **💚:** Remove duplicate headers on iOS ([#2665](https://github.com/Shopify/react-native-skia/issues/2665)) ([4e239fd](https://github.com/Shopify/react-native-skia/commit/4e239fdb0046348a859f76733484fe972cdb9db8))

### v1.4.0

#### Bug Fixes

- **⬆️:** Upgrade to Skia m130 ([#2648](https://github.com/Shopify/react-native-skia/issues/2648)) ([950bca7](https://github.com/Shopify/react-native-skia/commit/950bca7b47f0f0b2fd4e8aaa72bf91e7c3447725))
- **🌎:** componentProps prop to WithSkiaWeb ([#2662](https://github.com/Shopify/react-native-skia/issues/2662)) ([4c8fd33](https://github.com/Shopify/react-native-skia/commit/4c8fd33c1d2f422e0e3246a040857b20f0dcbbec))
- use TextDecorationStyle for SkTextStyle.decorationStyle ([#2628](https://github.com/Shopify/react-native-skia/issues/2628)) ([bb22157](https://github.com/Shopify/react-native-skia/commit/bb22157b3f11bdc08099867d32c3e681ac919f67))

#### Features

- **🔺:** experimental WebGPU Canvas integration ([#2632](https://github.com/Shopify/react-native-skia/issues/2632)) ([5c58cae](https://github.com/Shopify/react-native-skia/commit/5c58caeca9ba6251a5fdd4d1aaae627ccea3d037))

### v1.3.13

#### Bug Fixes

- **🐛:** yarn pack regression ([#2629](https://github.com/Shopify/react-native-skia/issues/2629)) ([25b1fb3](https://github.com/Shopify/react-native-skia/commit/25b1fb31095e0a96afe1ccc09d01d82571507898))

### v1.3.12

#### Bug Fixes

- **🍏:** bundler dep for 2020-36327 ([#2606](https://github.com/Shopify/react-native-skia/issues/2606)) ([a53ed23](https://github.com/Shopify/react-native-skia/commit/a53ed23ae73e4917dc5d3f7657f37b3ec0630e13))
- **🐛:** improve Canvas unmounting ([#2619](https://github.com/Shopify/react-native-skia/issues/2619)) ([d51beb3](https://github.com/Shopify/react-native-skia/commit/d51beb3ef8a0de9e7e360acaf1163076329e9d74))

### v1.3.11

#### Bug Fixes

- **🏞️:** add getFrameCount to animated images ([#2568](https://github.com/Shopify/react-native-skia/issues/2568)) ([682b32a](https://github.com/Shopify/react-native-skia/commit/682b32a74b1fbfe79e80f9603fa5dfc5c5ad01d8))
- **🤖:** Fix Android build on 0.75 ([#2586](https://github.com/Shopify/react-native-skia/issues/2586)) ([8e4bc1e](https://github.com/Shopify/react-native-skia/commit/8e4bc1eabb241f95f684f9a62d5a9576ccef81ee))
- add missing dispose and typename to skparagraph ([#2570](https://github.com/Shopify/react-native-skia/issues/2570)) ([9de8da3](https://github.com/Shopify/react-native-skia/commit/9de8da3ae5d1a30ab6c0dba6f4c0d615d24f8d2c))

### v1.3.10

#### Bug Fixes

- **🌉:** remove currentBridge ([#2548](https://github.com/Shopify/react-native-skia/issues/2548)) ([aadd424](https://github.com/Shopify/react-native-skia/commit/aadd424a5a9be215fee5f400ae0029a0180ad367))

### v1.3.9

#### Bug Fixes

- **🍏:** "Timed out waiting for modules to be invalidated" error after reloading app ([#2544](https://github.com/Shopify/react-native-skia/issues/2544)) ([45ae9cb](https://github.com/Shopify/react-native-skia/commit/45ae9cb3f1067b51958f2f1cdaa95a2ecd33f4de))
- **🤖:** imprecision in seek() ([#2532](https://github.com/Shopify/react-native-skia/issues/2532)) ([76343ce](https://github.com/Shopify/react-native-skia/commit/76343ce09385b9f4a7e996823f34f13771d34a96))

### v1.3.8

#### Bug Fixes

- **🐛:** Fix parameter handling in SkTypefaceFontProvider.matchFamilyStyle() ([#2518](https://github.com/Shopify/react-native-skia/issues/2518)) ([5665baa](https://github.com/Shopify/react-native-skia/commit/5665baa8735b04daf41cf8160c21575984525349))

