*[Информация на русском языке](#rus)*

# Cossacks: Back to War 1.42

<br/>

This repo contains the results of a severe refactoring and bugfixing of the original source code from Cossacks: Back to War 1.35 released back in 2002. The solution is meant to be build in Visual Studio 2015 on a Windows 10 system.

### Changelist
List of the most important bugs and crash causes that were fixed:
  * Unreasonable high game speed on modern CPUs
  * Messed up color palette 
  * Crashing when there are too many units on screen
  * Crashing when building very long walls
  * Crashing when selecting too much units at once
  * Multiple building explosions possible while performing upgrades
  * Instant building explosion upon repeated DEL order while performing an upgrade
  * DEL key killing selected units while entering a number in the resource transfer dialog
  * Buggy production queue management in multiple buildings at once
  * Grenadiers wouldn't shoot on attack move when in formation
  * Way too fast mouse scrolling
  * Crashing upon exiting a game
 
Main gameplay changes:
  * Stone mining upgrades now complement each other instead of overlapping
  * New option: Market (no trade; independent trade courses)
  * New option: Diplomatic Centre (no mercenaries allowed)
  * New option: Shipyard (no ships; fishing boats only; fishing boats and ferries; 17th century only)
  * Addition to option: No artillery **at all**
  * Game speed is set in the lobby and stays constant during the game
  * Production queue multiplicators: Shift (5), Alt (20), Tab (50), F1 (15), F2 (36) and F5 (250)
  * "Ready" flags are removed when the host changes settings in the lobby
  * "Click-guessing" the enemy's position through a fog of war bug is fixed away
  * No hills on "Plain land" maps anymore

Other improvements:
  * Windowed modes (activated through command line arguments /window and /borderless)
  * Default nation in lobby is set to "Random"
  * Minimap visibility improved through contrastful colors
  * Better handling of Ctrl+[0-9] key events
  * Game starts without UAC prompt
  * Menu resolution is fixed at original 1024x768, no more stretching
  * Upon clicking "Ready" in the lobby a list of all selected options is displayed
  * Screen resolutions fitting to your aspect ratio are marked with an asterisk
  * Confirm prompt added for surrendering and game exiting
  * Correct handling of DPI scaling settings
  * Upgrade research is canceled upon building capture
  * Fixed doubleclick "select all peasants on screen" bug while queueing buildings
  * Greatly increased the credits screen delay
  * Maximal screen resolution width is set to 1920x to mitigate impact of 4K displays
  * Removed the nagging "Unit / building xxx prevents you from shooting" messages
  * Increased the total amount and the lifetime of ingame chat messages from other players
  * Game recording is always on

<br/>

### Checksums
You could stumble upon an archive somewhere on the net… If you do, those could be helpful:

File|Size|SHA-1
:-|-:|:-
[Cossacks142.zip](https://github.com/ereb-thanatos/cossacks-revamp-2017/releases/download/1.42/Cossacks142.zip)|310,729,118|5b0dc863549db8a9d832c915dbc0d240681e82f3
dmcr.exe|1,975,296|3bb7d0c722d232b08033fa78ad5d4aefe3fc8900
IChat.dll|108,544|bcd17e3fe3fc242dfec5ec89497ba5863fe40729
IntExplorer.dll|114,176|310c721dfd259dd49c5905694981e9ebd78aa43c
resources.gsc|243,724,485|7522480fd6ccbffc1e5298b29fb7b12b5a54d9a0

You should also get the [Microsoft Visual C++ 2015 Redistributable (x86)](https://www.microsoft.com/en-us/download/details.aspx?id=53587) package.

<br/>

### Translation Patches
KaesDingeling kindly uploaded a [German translation patch](https://github.com/KaesDingeling/cossacks-revamp-2017/blob/master/override.zip).
Place the extracted **override.gsc** file in your game folder to activate it.

See [#19](https://github.com/ereb-thanatos/cossacks-revamp-2017/issues/19) for details on how to create your own translations.

<br/>
<br/>
<br/>

<a name="rus"></a>
# Казаки: Снова Война 1.42

<br/>

Здесь представлены результаты усердного рефакторинга и исправления исходного кода игры «Казаки: Снова Война» 1.35, вышедшей в 2002 году. Решение следует собирать в Visual Studio 2015 на системе Windows 10.

<br/>

### Список изменений
Самые важные из исправленных ошибок и вылетов:
  * Слишком высокая скорость игры на современных процессорах
  * Искажение цветовой палитры
  * Вылет при большом количестве юнитов на экране
  * Вылет при постройке длинных стен
  * Вылет при выборе множества юнитов
  * Многократный взрыв здания во время апгрейда
  * Моментальный взрыв здания при нажатии DEL во время апгрейда
  * Удаление юнитов при нажатии DEL в окне передачи ресурсов
  * Невозможность изменять очередь юнитов в нескольких зданиях сразу
  * Не стреляющие гренадёры в формациях
  * Слишком быстрый скроллинг
  * Вылет при выходе из игры
 
Основные изменения в геймплее:
  * Апгрейды на добычу камня дополняют друг друга согласно описанию
  * Новая опция: рынок (без обменов, независимые курсы)
  * Новая опция: дип. центр (игра без дип. центра)
  * Новая опция: верфь (без кораблей, только рыболовные, только рыболовные и транспорт, только корабли 17 века)
  * Дополнительная опция: совсем без артиллерии
  * Скорость игры настраивается в лобби и остаётся неизменной
  * Модификаторы очередей: Shift (5), Alt (20), Tab (50), F1 (15), F2 (36) and F5 (250)
  * Изменение настроек в лобби сбрасывает флаги готовности
  * Исправлен баг, позволяющий прокликивать сквозь туман войны
  * Убраны холмы из генерации ландшафта типа «равнины»

Прочие улучшения:
  * Режим окна и безрамного окна (ключи /window и /borderless)
  * Стандартная нация «случайная»
  * Контрастная миникарта
  * Исправлена обработка горячих клавиш Ctrl+[0-9]
  * Не требуются права администратора
  * Фиксированное разрешение главного меню
  * После нажатия «готов» показывается список всех настроек игры
  * Подходящие разрешения экрана помечаются звёздочкой
  * Диалог подтверждения перед тем, как сдаться или выйти в меню
  * Правильная работа игры с включённым DPI scaling
  * При захвате здания отменяется исследование апгрейда
  * При строительстве зданий двойной щелчок больше не выбирает всех крестьян
  * Задержка перед включением титров значительно увеличена
  * Разрешение экрана ограничено шириной в 1920px
  * Убрано сообщение «[юнит|здание] мешает выстрелу»
  * Увеличено количество и время показа сообщений от других игроков
  * Запись игры всегда включена

<br/>

### Контрольные суммы
Если вы вдруг наткнётесь на какой-то архив на просторах сети, то вам может понадобиться это:

Файл|Размер|SHA-1
:-|-:|:-
[Cossacks142.zip](https://github.com/ereb-thanatos/cossacks-revamp-2017/releases/download/1.42/Cossacks142.zip)|310.729.118|5b0dc863549db8a9d832c915dbc0d240681e82f3
dmcr.exe|1.975.296|3bb7d0c722d232b08033fa78ad5d4aefe3fc8900
IChat.dll|108.544|bcd17e3fe3fc242dfec5ec89497ba5863fe40729
IntExplorer.dll|114.176|310c721dfd259dd49c5905694981e9ebd78aa43c
resources.gsc|243.724.485|7522480fd6ccbffc1e5298b29fb7b12b5a54d9a0

Также вам понадобится [Распространяемый компонент Microsoft Visual C++ 2015 (x86)](https://www.microsoft.com/ru-ru/download/details.aspx?id=53587).
