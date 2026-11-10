# 2026-11-10 — еженедельная проверка SDK и точной совместимости 1.26.33

## Краткий итог
Найден новый полезный материал именно для Bedrock **1.26.33.1 / Android arm64-v8a**: сторонний проект GlowberryClient опубликовал статический разбор libminecraftpe.so с тем же SHA-256, что и исследовавшаяся целевая библиотека. Это даёт два подтверждённых статически vtable-ориентира, но не готовый ECS/entity feed и не подтверждённую на устройстве матрицу ViewProjection. Сам проект использует отдельный launcher/runtime, поэтому его готовую сборку нельзя считать совместимой с LeviLauncher 1.5.13.

## 1. Точное совпадение бинарника
В ChimeraAnt-DEV/GlowberryClient опубликован разбор docs/binary-analysis-1.26.33.1.md. В нём указаны:
- версия APK: 1.26.33.1;
- ABI: arm64-v8a;
- размер libminecraftpe.so: 349,243,744 байт;
- SHA-256: d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1.

Хэш совпадает с ранее зафиксированным хэшем целевой библиотеки. Это делает анализ релевантным именно этому бинарнику, а не соседним 1.26.50/1.26.60.

Источник: https://github.com/ChimeraAnt-DEV/GlowberryClient/blob/main/docs/binary-analysis-1.26.33.1.md
Методика/результат добавлены в коммите 17d225952ed050200da34274f71c1442f2ca1810 (2026-10-15): https://github.com/ChimeraAnt-DEV/GlowberryClient/commit/17d225952ed050200da34274f71c1442f2ca1810

## 2. Что подтверждено статически
- LivePlayerRenderer: vtable address point 0x1404a318, слот 17, цель 0x9c4a78c. В документе слот подтверждён по RTTI/vtable и ссылкам на variable.player_x_rotation.
- ClientInstance: vtable address point 0x140545a0, слот 31, цель 0x9d72118 — кандидат на accessor локального игрока.
- Эти факты являются результатами статического анализа. Сам автор отдельно отмечает, что вызов хука на устройстве ещё не доказан, а ABI аргументов LivePlayerRenderer::render не расшифрован.

Не использовать найденные слоты как доказательство готового ESP: это только render/local-player ориентиры.

## 3. ECS, хитбоксы и матрица камеры
- StateVectorComponent, HitboxComponent, AABBShapeComponent: в найденном точном разборе нет подтверждённых полевых offsets или рабочего способа получить список всех сущностей.
- Отдельная заметка проекта прямо указывает, что нативный EntitySource не установлен; hitbox-данные мобов/предметов и другие entity-dependent функции не имеют подтверждённого источника. Peer-only боксы игроков из voice feed — это не общий игровой entity feed.
- Локальный игрок — отдельный путь через ClientInstance slot 31. Поля Actor+0x230 (position) и Actor+0x238 (rotation) в основном binary-analysis документе помечены как непроверенные на 1.26.33.1. Упоминание этих смещений в соседнем исследовательском документе опирается на проверки других сборок, поэтому переносить их как подтверждённые offsets для целевого бинарника нельзя.
- ViewProjection: проект использует CameraSource / matrix scan, но точный разбор 1.26.33.1 прямо помечает источник реальной view-projection как невыведенный и непроверенный. Нужна диагностика на устройстве, отличающая реальную engine matrix от first-person fallback.

Источники:
- https://github.com/ChimeraAnt-DEV/GlowberryClient/blob/main/docs/agents/native-entity-camera-feed-what-is-and-is-not-reachable.md
- https://github.com/ChimeraAnt-DEV/GlowberryClient/blob/main/docs/MODULE_STATUS.md
- https://github.com/ChimeraAnt-DEV/GlowberryClient/blob/main/docs/DEVICE_CHECKLIST.md

## 4. SDK / Preloader / launcher
- Официальная Android-документация LeviLauncher описывает native mods через LiteLDev/preloader-android и pl:: API; developer checklist требует ABI arm64-v8a. Это подтверждает путь сборки Android native mod, но само по себе не подтверждает game offsets для 1.26.33.1.
  - https://docs.levimc.org/LeviLaunchroid/guide/build
  - https://docs.levimc.org/LeviLaunchroid/api/mod
- В LiteLDev/preloader-android последняя проверенная запись основной ветки — commit cc08b40a31471d3e2dedb267f5f17b28da2b0604 от 2026-11-05: оптимизация pattern scanning. В сообщении нет заявления о поддержке именно Minecraft 1.26.33.1.
  - https://github.com/LiteLDev/preloader-android/commit/cc08b40a31471d3e2dedb267f5f17b28da2b0604
  - Публичная страница релизов показывает SDK tag 0.2.3 как latest; это версия Preloader SDK, не Minecraft build: https://github.com/LiteLDev/preloader-android/releases
- Официальный LeviLauncher Android уже выпустил v1.6.1 2026-11-06; v1.6.0 от 2026-10-27 отдельно объявлял поддержку Minecraft 26.60. Это не подтверждение совместимости старого LeviLauncher 1.5.13 с конкретными offsets для 1.26.33.1.
  - https://github.com/LiteLDev/LeviLauncher-Android/releases
- Текущий QYCottage/BedrockTools manifest — версия 1.5.12, minecraft_versions: 1.26.6X.X; последний version-bump commit датирован 2026-11-07. Поэтому по собственному manifest проект не заявляет поддержку 1.26.33.
  - https://github.com/QYCottage/BedrockTools/blob/main/levimod.json
  - https://github.com/QYCottage/BedrockTools/commit/c03d3ae0c9022557de0acdf0d94e373e0b9e0579

## Вывод для целевой конфигурации
| Область | Статус |
|---|---|
| Android ARM64 Preloader API | Есть публичный официальный SDK/API |
| LeviLauncher 1.5.13 + точный Preloader commit | Не подтверждено в проверенных первоисточниках |
| BedrockTools для 1.26.33 | Не заявлена; manifest указывает 1.26.6X.X |
| LivePlayerRenderer slot 17 | Статически подтверждён для точного бинарника |
| ClientInstance slot 31 | Статически подтверждён для точного бинарника |
| StateVectorComponent / HitboxComponent / AABBShapeComponent | Рабочий доступ и offsets не подтверждены |
| Entity list / все игроки, мобы и dropped items | Не подтверждён |
| Реальная ViewProjection matrix на 1.26.33.1 | Не подтверждена; требуется device diagnostics |

## Следующий шаг
Использовать точный бинарный разбор как источник для дальнейшей проверки render path, но не копировать чужой launcher-specific Preloader runtime. Отдельно получить точный Preloader submodule commit, который был упакован именно в LeviLauncher 1.5.13, и проверить на устройстве slot 17, slot 31 и camera diagnostics. До подтверждения entity iteration и матрицы камеры рабочую 3D ESP-библиотеку считать неготовой.
