# Лабораторная Работа №4. Управление памятью
## Цель работы
Знакомство с принципами организации виртуальной памяти

## Справочный материал

<!-- ### 0. Virtual memmory -->

### 1. Таблицы страниц
Виртуальный адрес состоит из двух частей: номера виртуальной страницы и смещения относительно начала этой страницы. Обычно в 32-битной системе страницы имеют размер 4KB, откуда следует, что для смещения используется 12 битов (2<sup>12</sup> = 4096), а для номера страницы - 20 бит. Для  преобразования такого виртуального адреса в физический адрес каждому процессу необходимо иметь таблицу страниц, которая устанавливает соответствие между виртуальными страницами и физическими страничными блоками. Во время преобразования адреса, диспетчер памяти ищет в таблице страниц нужную виртуальную страницу и заменяет ее номер на номер соответствующего физического страничного блока, оставляя при этом смещение неизменным. <!-- см. рисунок --> Запись в таблице страниц также содержит дополнительную информацию, такую как уровень защиты страницы (чтение, запись, выполнение), биты обращения, изменения, присутствия и др. Ситуация, когда искомая страница отсутствует в физической памяти (бит присутствия в таблице страниц равен нулю), называется ошибкой отсутствия страницы (page fault). <!-- пример трансляции адреса в виде таблицы -->

Использование страничной организации памяти не лишено недостатков. Процесс может выделять память только фрагментами, кратными размеру страницы. В результате могут образовываться страницы, большая часть памяти в которых не используется. Эта ситуация называется *внутренней фрагментацией*. Для решения этой проблемы можно использовать страницы меньшего размера, но это приводит к росту размера таблицы страниц. Действительно, рассмотрим 32-битное адресное пространство, разделенное на страницы размером 4KB. 20 бит отводятся под номер страницы, что соответствует 2<sup>20</sup> = 1M записей в таблице страниц. Каждая запись в таблице страниц содержит 20-битный номер соответствующей ей физической страницы и несколько дополнительных битов со вспомогательной информацией, всего около 32 бит. Таким образом, размер таблицы страниц составляет 1M * 32 бита = 4MB, причем все эти 4MB должны располагаться в физической памяти, иначе, при загрузке таблицы страниц с жесткого диска, преобразование адреса будет занимать слишком много времени. Поскольку каждый процесс имеет свою собственную таблицу страниц, при использовании страничной организации памяти в 32-битной системе необходимо выделять по 4MB оперативной памяти для хранения таблицы страниц каждого процесса. При большом количестве запущенных процессов это может привести к ситуации, когда таблицы страниц занимают существенную часть физической памяти.

<!-- ### 2. Multilevel paging -->

### 2. Инвертированные таблицы страниц
#### 2.1. Введение
Рассмотрим систему, в которой виртуальное адресное пространство состоит из 64-битных адресов, размер страниц равен 4KB, а объем доступной физической памяти составляет 512MB. Сколько места в памяти потребуется для простой одноуровневой таблицы страниц? Такая таблица содержит по одной записи на каждую виртуальную страницу, т.е. 2<sup>64</sup>/2<sup>12</sup> = 2<sup>52</sup> записей. Каждая запись в таблице страниц будет занимать около 4 байт, поэтому общий размер таблицы страниц составит 2<sup>54</sup> байт или 16 петабайт (*пета-* > *тера-* > *гига-*)! И такая таблица страниц будет у **каждого** процесса.

Конечно, процесс навряд ли будет использовать все 64 бита адресного пространства, поэтому можно было бы попробовать использовать многоуровневые таблицы страниц. Сколько уровней таблицы страниц потребуется, если отдельная таблица на каждом уровне должна помещаться на одной виртуальной странице памяти, т.е. занимать не более 4KB? Пусть отдельная запись (строка) в таблице страниц имеет длину 4 байта, тогда таблица страниц на каждом уровне может содержать 1024 записи или 10 бит адресного пространства. Получается, многоуровневая таблица страниц будет иметь `[52/10] = 6` уровней. Но это значит, что каждая трансляция адреса будет приводить к 6 операциям доступа к памяти, т.е. производительность упадет в 6 раз по сравнению с простой одноуровневой таблицей страниц.

Можно заметить, что объем физической памяти, установленной в системе, значительно меньше, чем размер виртуального адресного пространства: 512MB = 2<sup>29</sup>B << 2<sup>64</sup>B. Или, в пересчете на страничные блоки: 2<sup>29</sup>/2<sup>12</sup> = 2<sup>17</sup> = 128K страничных блоков. Если хранить в таблице страниц по одной записи для каждого страничного блока, а не для каждой виртуальной страницы каждого процесса, то размер таблицы страниц существенно сократится. Например, при длине одной записи в такой таблице страниц в 16 байт, вся таблица займет лишь 2MB. Поскольку все процессы используют одну и ту же физическую память, потребуется всего лишь одна единственная подобная таблица. Эта концепция называется *инвертированной таблицей страниц*.

#### 2.2. Линейная инвертированная таблица страниц
Простейшая инвертированная таблица страниц содержит по одной записи для каждого страничного блока физической памяти в виде линейного массива последовательных записей. Поскольку таблица является общей для всех процессов, каждая запись должна содержать идентификатор процесса, владеющего страницей. А по скольку страничные блоки теперь отображаются на виртуальные страницы, каждая запись содержит номер виртуальной страницы вместо физической. Номер физической страницы (страничного блока) в явном виде не хранится, т.к. он совпадает с порядковым номером записи (индексом массива). В таблице также присутствуют стандартные биты с дополнительной информацией об уровне защиты страницы и учете недавних операций доступа к ней. Если идентификатор процесса имеет длину в 16 бит, еще 52 бита отводятся под номер виртуальной страницы, а 12 бит - под дополнительную информацию, то каждая запись в инвертированной таблице страниц будет занимать 80 бит или 10 байт, что дает общий размер таблицы в 10*128KB = 1.3MB.

Для трансляции виртуального адреса в физический, номер виртуальной страницы и идентификатор текущего процесса поочередно сравниваются с каждой из записей в инвертированной таблице страниц. Если совпадение найдено, индекс совпавшей записи заменяет номер виртуальной страницы и тройка значений `<идентификатор процесса; индекс в инвертированной таблице страниц; смещение>` образует физический адрес. Если совпадений нет, генерируется ошибка отсутствия страницы. <!-- Эта процедура трансляции адреса показана на рисунке. -->

Несмотря на то, что инвертированная таблица страниц имеет относительно небольшой размер, последовательный поиск по ней может быть очень медленным. В худшем случае для того, что бы найти совпадение, потребуется просмотреть всю таблицу целиком, что равносильно 128K доступам к памяти. В среднем можно ожидать 64K доступов к памяти для трансляции адреса. Это слишком неэффективно, поэтому, чтобы уменьшить время поиска, используется хэширование.

#### 2.3. Инвертированная таблица страниц с хэшированием
Данный подход добавляет еще один уровень непосредственно перед самой таблицей страниц - хэш-таблицу. Эта хэш-таблица должна иметь как минимум столько же записей, сколько и таблица страниц. Она ставит в соответствие паре значений `<идентификатор процесса; номер виртуальной страницы>` запись в инвертированной таблице страниц. Поскольку при использовании хэш-функции возможны коллизии, необходимо использовать связанные списки для каждой из записей в хэш-таблице. <!-- Если в хэш-таблице будет столько же строк, сколько физических страниц (страничных блоков) имеется в данной ЭВМ, то в среднем список будет иметь длину в одну запись. --> Каждый элемент списка соответствует физической странице и имеет соответствующую ей запись в инвертированной таблице страниц, а весь список может быть представлен как последовательность записей в таблице страниц, где каждая запись имеет указатель на следующую запись из списка. Для реализации такого подхода потребуются дополнительные 4 байта для каждой записи в таблице страниц, после чего размер таблицы страниц составит 14 * 128KB = 1.8MB. Хэш-таблица занимает дополнительные 4 * 128KB = 512KB. Итого общие накладные расходы составят 2.3MB.

Для преобразования виртуального адреса в физический, идентификатор процесса и номер виртуальной страницы подаются на вход хэш-функции, значение которой позволяет найти нужную запись в хэш-таблице. Затем осуществляется переход в таблицу страниц по указателю из записи в хэш-таблице. Идентификатор процесса и номер виртуальной страницы сравниваются с записью в таблице страниц. Если они не совпали, осуществляется переход по указателю на следующую страницу, указанную в найденной записи в инвертированной таблице страниц, и процедура повторяется, пока не будет найдено совпадение. Если достигнут конец цепочки (указатель на следующую страницу равен нулю), генерируется ошибка отсутствия страницы. <!-- Эта процедура показана на рисунке -->

При условии использования хорошей хэш-функции, средняя длина списка будет порядка 1.5 элементов. Значит, в среднем потребуется всего лишь 2.5 доступа к памяти для преобразования адреса. Это довольно хороший результат, учитывая, что двух-уровневая таблица страниц требует 2 доступов к памяти. Однако это не предел и время, необходимое для трансляции адреса, может быть уменьшено еще сильнее за счет использования кэширования с помощью буфера быстрого преобразования адреса TLB (Translation Lookaside Buffer).

### 3. TLB
См. [Буфер ассоциативной трансляции](https://ru.wikipedia.org/wiki/Буфер_ассоциативной_трансляции) и [Translation lookaside buffer](https://en.wikipedia.org/wiki/Translation_lookaside_buffer)

### 4. Алгоритмы замещения страниц

#### 4.1. Первым пришел - первым обслужен (FIFO)
Данный алгоритм является одним из самых простых в реализации. Операционная система ведет очередь загруженных в память страниц. При загрузке в память новой страницы она добавляется в конец очереди. При возникновении ошибки отсутствия страницы место под новую страницу высвобождается путем удаления страницы из головы очереди.

#### 4.2. Второй шанс (Second chance)
Модификация алгоритма FIFO. Если при возникновении ошибки отсутствия страницы в памяти у страницы, находящейся в голове очереди, бит использования `R` установлен (т.е. равен `1`), то этот бит сбрасывается в ноль, а сама страница перемещается в конец очереди. После этого проверяется следующая страница, оказавшаяся в голове очереди. Процедура повторяется до тех пор, пока в голове очереди не окажется страница, у которой бит использования не установлен (т.е. равен нулю). Эта страница выгружается из памяти и на ее место загружается новая.

#### 4.3. Часы (Clock)
Дальнейшая модификация алгоритма Второй шанс. Поскольку перемещать страницы из головы очереди в конец весьма неэффективно, для увеличения скорости работы алгоритма Второй шанс очередь может быть реализована в виде циклического списка. В этом случае фактического перемещения страниц из головы в конец очереди не происходит. Вместо этого изменяется значение переменной, указывающей на голову очереди. В этом алгоритме  переменная-указатель ("стрелка" часов) всегда указывает на голову списка, т.е. на страницу, загруженную раньше всех остальных.

#### 4.4. Случайный выбор (Random)
Выбор замещаемой страницы осуществляется с помощью датчика случайных чисел, имеющего равномерное распределение. Таким образом, при возникновении ошибки отсутствия страницы вероятность быть выгруженной для всех страниц одинакова.

#### 4.5. Не использовавшаяся в последнее время страница (NRU, Not Recently Used)
В соответствии со значениями битов использования и модификации (`R` и `M`), каждый страничный блок относят к одному из 4-х классов:
- класс 0, страница не использовалась и не была модифицирована;
- класс 1, страница не использовалась, но была модифифирована;
- класс 2, страница использовалась, но не была модифицирована;
- класс 3, страница использовалась и была модифицирована.

Удаляют страницу, относящуюся к классу с наименьшим номером. Если таких страниц несколько, одну из них выбирают случайным образом.

#### 4.6. Наименее востребованная страница (LRU, Least Recently Used)
Все страницы сортируются в порядке обращения к ним. В голове списка страница, к которой обращение было последним (только что). В конце списка страница, к которой обращение было раньше всех остальных (давно). При каждом обращении к памяти список перестраивается. При возникновении ошибки отсутствия страницы удаляется страница из конца очереди.

#### 4.7. Не часто востребованная страница (NFU, Not Frequently Used)
Модификация алгоритма LRU, призванная повысить его производительность за счет отказа от перестановки элементов списка при каждой операции обращения к памяти. Вместо этого используется счетчик для каждой из страниц, начальное значение которого равно нулю. При каждом прерывании таймера операционная система добавляет к значению счетчика каждой страницы значение бита использования (`R`) этой страницы. В случае возникновения ошибки отсутствия страницы, удаляется та страница, значение счетчика которой минимально. Если имеется несколько страниц с одинаковым минимальным значением счетчика, необходимо выбрать одну из них случайным образом.

#### 4.8. Старение (Aging)
Дальнейшая модификация алгоритмов LRU и NFU. Для каждой страницы, загруженной в память, заводится счетчик. По прерыванию от таймера счетчики всех страниц сдвигаются вправо на 1 бит. Затем, самый старший (левый) бит счетчика устанавливается равным значению бита использования (`R`) этой страницы. В случае возникновения ошибки отсутствия страницы, удаляется та страница, значение счетчика которой минимально. Если имеется несколько страниц с одинаковым минимальным значением счетчика, необходимо выбрать одну из них случайным образом.

#### 4.9. Рабочий набор (Working set)
При возникновении ошибки отсутствия страницы ОС просматривает всю таблицу страниц (страничных блоков). Для каждой страницы, у которой бит использования установлен (`R = 1`), в отдельное поле в таблице страниц (специальный счетчик) записывается текущее системное время. Далее вычисляется "возраст" каждой страницы как разность между текущим значением системного времени и значением в отдельном поле (счетчике) этой страницы. Удаляется страница с наибольшим возрастом. Если в течение последнего интервала таймера были обращения ко всем страницам, и, соответственно, возраст у всех страниц равен нулю, случайным образом выбирается для удаления одна из страниц, у которой бит использования не установлен (`M = 0`). Если же у всех страниц, находящихся в памяти, биты R и M установлены, то случайным образом удаляется произвольная страница. Для страницы, которая вызвала ошибку отсутствия страницы и была загружена на место удаленной, значение счетчика устанавливается равным текущему системному времени.


## Задание

В данной работе необходимо реализовать фрагмент диспетчера памяти и часть функционала операционной системы, отвечающего за замещение страниц при возникновении ошибок отсутствия страниц. Для упрощения работы предполагается использование линейной инвертированной таблицы страниц, работу с которой необходимо реализовать в виде программы. Также для простоты предполагается, что в системе имеется один единственный процесс, поэтому идентификатор процесса в инвертированной таблице страниц не хранится. Входные данные представляют собой последовательность операций обращения к памяти, выходные данные - состояние инвертированной таблицы страниц после каждой операции обращения к памяти.

1.  Вычислить номер варианта по списку в журнале и сохранить его в файл [`TASKID.txt`](TASKID.txt) в репозитории.
2.  Написать программу на языке C++ в соответствии со следующей спецификацией.
    1. Входные данные:
         1. Аргумент командной строки (число): номер алгоритма замещения страниц, который должна использовать программа. Принимает значения `1` или `2`, соответствующие двум алгоритмам замещения страниц, заданным по варианту.
         2. Перечень инструкций обращения к памяти, считываемый программой из стандартного потока ввода. На каждой строке не более одной инструкции. Инструкция состоит из двух чисел, разделенных пробелом, например: `0 1`. Первое число обозначает тип операции доступа к памяти: `0` - чтение и `1` - запись. Второе число является номером виртуальной страницы, к которой происходит обращение.
    2. Выходные данные:
         1. Для каждой операции обращения к памяти, информация о которой поступила на вход программы, на выходе должна быть сгенерирована строка, содержащая содержимое инвертированной таблицы страниц в виде последовательности номеров виртуальных страниц, разделенных пробелом. Если какая-либо из записей в таблице страниц отсутствует (таблица страниц не заполнена до конца), вместо номера виртуальной страницы необходимо вывести символ `#`.

3. Весь код поместить в файле `lab4.cpp`. Код должен корректно компилироваться командой `g++ lab4.cpp -o lab4 -std=c++11`. Настоятельно рекомендуется использовать стандартную библиотеку STL. Полезными могут быть контейнеры [`list`](https://en.cppreference.com/w/cpp/container/list), [`vector`](https://en.cppreference.com/w/cpp/container/vector), [`bitset`](https://en.cppreference.com/w/cpp/utility/bitset) и др. 
4. Если в работе алгоритма замещения страниц используется бит `R`, то необходимо реализовать эмуляцию прерывания таймера. Для этого через каждые 5 операций обращения к памяти необходимо запускать обработчик данного прерывания. Значения битов `R` по прерыванию таймера сбрасываются.
5. Для алгоритмов, использующих счетчик (NFU, Aging): если несколько страниц имеют одинаковое значение счетчика, одна из них выбирается случайным образом. При повторной загрузке страницы в память ее счетчик обнуляется. В алгоритме старения счетчик имеет размер 1 байт. В алгоритме NFU счетчик имеет размер не меньше 4 байт.
6. Во всех алгоритмах, использующих датчик случайных чисел (Random, NRU, NFU, Aging, ...), разрешается использовать **только** функцию `int uniform_rnd(int a, int b)`, объявленную в файле [`lab4.h`](lab4.h). Данная функция генерирует случайное целое число с равномерным распределением из диапазона `[a, b]`. Использование других функций для работы со случайными числами запрещено!
7. В качестве системного времени в алгоритме рабочего набора следует использовать количество инструкций доступа к памяти, обработанных с момента запуска программы.
8. После успешного прохождения локальных тестов необходимо загрузить код в репозиторий на гитхабе.
9. Сделать выводы об эффективности реализованных алгоритмов замещения страниц. Сравнить количество ошибок отсутствия страниц, генерируемых на тестовых данных при использовании каждого алгоритма.
10. Подготовить отчет о выполнении лабораторной работы и загрузить его под именем `report.pdf` в репозиторий. В случае использования системы компьютерной верстки LaTeX также загрузить исходный файл `report.tex`.

<!-- Алгоритмы.
+FIFO
+Random
+Second chance (FIFO extension with R bit)
+Clock (Second chance extension with circular list)
LRU (linked list reordered on each memory access, extremely slow)
+NFU (R bit added to a counter on each timer interrupt)
+Aging (shift right, than add R bit to counter on each timer interrupt)
+NRU (4 класса по битам R и M)

Для алгоритмов, использующих счетчик (NFU, Aging): если несколько страниц имеют одинаковое значение счетчика, одна из них выбирается случайным образом. При повторной загрузке страницы в память ее счетчик обнуляется. 
В Aging счетчик имеет размер 1 байт. В NFU счетчик должен меть размер 8 байт.
Для генерации случайных чисел использовать только функцию `int uniform_rnd(int a, int b)`, объявленную в файле `lab4.h`.

Working Set
WSClock (Working Set Clock)
-->

## Комментарии и примеры

### Структуры данных и алгоритмы
Упрощенная линейная инвертированная таблица страниц, которую необходимо использовать в данной лабораторной работе, имеет следующую структуру:

| PPN | VPN | `R` | `M` | Счетчик |
| --: | --: | --: | --: |   --:   |
|  0 | 21 | 0 | 0 | `0b00001001` | 
|  1 |  7 | 0 | 1 | `0b01000000` | 
|  2 | 12 | 1 | 0 | `0b10000000` | 
|  3 |  0 | 1 | 1 | `0b00000010` | 
|  4 |  1 | 0 | 0 | `0b01000000` | 
|  5 | 34 | 1 | 0 | `0b00000001` | 
|  6 | 33 | 0 | 0 | `0b00000000` | 
|  7 |  - | - | - |            - | 

где 
- PPN, physical page number - номер физической страницы (страничного блока); в явном виде может не храниться, поскольку совпадает с номером записи в таблице;
- VPN, virtual page number - номер виртуальной страницы;
- `R` - бит использования (обращения);
- `M` - бит модификации;
- Счетчик - дополнительное числовое поле, которое может использоваться в алгоритмах NFU, Aging, Working set; здесь в качестве примера показаны возможные двоичные значения 8-битного счетчика при использовании алгоритма старения.

Программная реализация данной таблицы, в зависимости от используемого алгоритма замещения страниц, может быть осуществлена в виде массива, списка, очереди, а также в виде комбинации из этих и других структур данных. Работы с отдельными битами можно избежать воспользовавшись булевыми переменными, т.к. моделируемые таблицы страниц имеют небольшой размер и необходимость в экономии памяти отсутствует. Также для этих целей можно использовать шаблон стандартной библиотеки [`bitset`](https://en.cppreference.com/w/cpp/utility/bitset).


### Пример входных и выходных данных
Пусть имеется ЭВМ с некоторым количеством ОЗУ и установленной операционной системой, использующей механизм виртуальной памяти. Доступная в ОЗУ физическая память разделена на 5 страничных блоков. Операционная система или пользователь запускает процесс. После запуска процесса он последовательно обращается к адресам в памяти, расположенным на следующих виртуальных страницах (второй столбец) для выполнения операций чтения (`0` в первом столбце) или записи (`1` в первом столбце):
```
0 0
0 1
0 2
0 3
0 4
0 5
0 0
1 0
0 5
1 0
```

Порядок заполнения таблицы страниц в этом случае может выглядеть так:
```
0 # # # #
0 1 # # #
0 1 2 # #
0 1 2 3 #
0 1 2 3 4
5 1 2 3 4
5 0 2 3 4
5 0 2 3 4
5 0 2 3 4
5 0 2 3 4
```
Здесь каждая строка представляет собой содержимое таблицы страниц после выполнения операции доступа к памяти. Символ `#` означает пустой страничный блок. Изначально вся инвертированная таблица страниц является пустой (ни один страничный блок не используется), что может быть представлено как `# # # # #`. На шестом обращении к памяти (к виртуальной странице `5`) происходит ошибка отсутствия страницы и запускается алгоритм замещения страниц, поскольку свободных мест в инвертированной таблице страниц больше нет. Этот алгоритм выселяет из памяти виртуальную страницу `0` и помещает на ее место запрошенную виртуальную страницу `5`. На следующем обращении к памяти запрашивается только что удаленная виртуальная страница `0` и ее приходится заново загрузить в память, предварительно удалив виртуальную страницу `1`. Последующие обращения к памяти не приводят к ошибкам отсутствия страниц и на первый взгляд состояние таблицы страниц не меняется. На самом деле это не так: в отличие от предыдущих обращений к памяти, которые не изменяли данные, восьмой и десятый запросы изменяют данные на виртуальной странице `0`. Следствием этого является установка бита `M` для данной страницы. Также через каждые несколько обращений к памяти происходит эмуляция прерывания таймера и биты `R` всех страниц сбрасываются. В рамках задания на данную лабораторную работу необходимо вывести только номера виртуальных страниц, находящихся в инвертированной таблице страниц. Значения битов `R` и `M` не отображаются, но могут учитываться для корректной работы того или иного алгоритма замещения страниц.

Выше показаны примеры входных и выходных данных для программы, которую необходимо разработать в рамках данной лабораторной. Рассмотрим также пример того, как могут выглядеть данные для отладки программы.  
**NB!** Ниже представлены примеры выходных данных, которые могут использоваться для отладки работы алгоритмов, но не могут являться окончательным результатом работы программы, т.к. не соответствуют описанию выходных данных.
```
0(1,0) #(0,0) #(0,0) #(0,0) #(0,0)
0(1,0) 1(1,0) #(0,0) #(0,0) #(0,0)
0(1,0) 1(1,0) 2(1,0) #(0,0) #(0,0)
0(1,0) 1(1,0) 2(1,0) 3(1,0) #(0,0)
0(0,0) 1(0,0) 2(0,0) 3(0,0) 4(0,0)
5(1,0) 1(0,0) 2(0,0) 3(0,0) 4(0,0)
5(1,0) 0(1,0) 2(0,0) 3(0,0) 4(0,0)
5(1,0) 0(1,1) 2(0,0) 3(0,0) 4(0,0)
5(1,0) 0(1,1) 2(0,0) 3(0,0) 4(0,0)
5(0,0) 0(0,1) 2(0,0) 3(0,0) 4(0,0)
```
В данном примере после номера виртуальной страницы в скобках через запятую указаны значения битов `R` и `M`. При загрузке новой страницы, а также при обращении к уже находящейся в памяти странице, ее бит `R` устанавливается в единицу. Если обращение к странице осуществляется для записи данных, то в единицу также устанавливается бит `M`. В данном примере эмуляция прерывания по таймеру происходит после каждого пятого обращения к памяти и приводит к обнулению битов `R` всех загруженных в память страниц.

В рассмотренном выше примере алгоритм замещения страниц не использовал счетчик, поэтому при отладке его значение не выводилось. Рассмотрим теперь пример того, как могут выглядеть данные для отладки программы при использовании другого алгоритма замещения страниц, а именно - алгоритма старения. Для каждой записи в таблице страниц добавлено значение 8-битного счетчика, которое может помочь при отладке программы.
```
0(1,0,0b00000000) #(0,0,0b00000000) #(0,0,0b00000000) #(0,0,0b00000000) #(0,0,0b00000000)
0(1,0,0b00000000) 1(1,0,0b00000000) #(0,0,0b00000000) #(0,0,0b00000000) #(0,0,0b00000000)
0(1,0,0b00000000) 1(1,0,0b00000000) 2(1,0,0b00000000) #(0,0,0b00000000) #(0,0,0b00000000)
0(1,0,0b00000000) 1(1,0,0b00000000) 2(1,0,0b00000000) 3(1,0,0b00000000) #(0,0,0b00000000)
0(0,0,0b10000000) 1(0,0,0b10000000) 2(0,0,0b10000000) 3(0,0,0b10000000) 4(0,0,0b10000000)
0(0,0,0b10000000) 1(0,0,0b10000000) 5(1,0,0b00000000) 3(0,0,0b10000000) 4(0,0,0b10000000)
0(1,0,0b10000000) 1(0,0,0b10000000) 5(1,0,0b00000000) 3(0,0,0b10000000) 4(0,0,0b10000000)
0(1,1,0b10000000) 1(0,0,0b10000000) 5(1,0,0b00000000) 3(0,0,0b10000000) 4(0,0,0b10000000)
0(1,1,0b10000000) 1(0,0,0b10000000) 5(1,0,0b00000000) 3(0,0,0b10000000) 4(0,0,0b10000000)
0(0,1,0b11000000) 1(0,0,0b01000000) 5(0,0,0b10000000) 3(0,0,0b01000000) 4(0,0,0b01000000)
```
Следует отметить, что данный слепок таблицы страниц, полученный по окончании работы программы, отличается от показанного в предыдущем примере. Использование иного алгоритма замещения страниц повлияло на то, какие именно страницы были выгружены. В данном случае на шестом обращении к памяти к виртуальной странице `5` все страницы имели одинаковый возраст и случайным образом для выселения была выбрана виртуальная страница `2`.

## Сборка и тестирование
Программа должна компилироваться командой `g++ lab4.cpp -o lab4 -std=c++11`. Тесты запускаются командой `./tests.sh`. Для отладки программы рекомендуется выводить на каждом шаге дополнительную информацию о состоянии таблицы страниц: значения битов использования и модификации, значение счетчика и т.п. Перед запуском тестов вывод избыточной отладочной информации следует отключить.

## Рейтинг
Задание может быть выполнено неполностью с соответствующей потерей рейтинга. Можно реализовать только один из двух алгоритмов замещения страниц, указанных по варианту. В этом случае количество баллов за данную лабораторную работу будет уменьшено вдвое. Чтобы успешно пройти тесты при неполном выполнении лабораторной работы необходимо создать в корне репозитория файл `ALGORITHM.txt` и записать в него номер алгоритма (`1` или `2`), который был реализован в рамках данной лабораторной работы. Если все сделано верно, при запуске тестов командой `./tests.sh` будет осуществлена проверка работоспособности только одного из двух алгоритмов, номер которого был найден в файле. После успешного прохождения тестов одним алгоритмом необходимо выполнить команды `git add ALGORITHM.txt`, `git commit`, `git push`. После этого доделать второй алгоритм и получить за него баллы будет уже нельзя.

В случае, если работа выполнена полностью и оба алгоритма замещения страниц реализованы, никаких дополнительных действий предпринимать не нужно. Для получения максимального количества баллов за данную лабораторную работу файл `ALGORITHM.txt` в репозитории должен **отсутствовать**.

## Содержание отчета
- Титульный лист 
- Цель работы
- Задание на лабораторную работу
- Описание используемых алгоритмов замещения страниц
- Результат выполнения работы
- Исходный код программы с комментариями
- Выводы


## Варианты заданий
| Номер варианта | Количество страничных блоков | Алгоритм 1 | Алгоритм 2 |
| ---:           |           ------:           |           ------:           |         ------:       |
|  1 |  8 | FIFO          | Aging |
|  2 |  6 | Second chance | NFU |
|  3 |  8 | Clock         | NRU |
|  4 |  6 | Random        | LRU |
|  5 |  6 | FIFO          | NFU |
|  6 |  8 | Second chance | NRU |
|  7 |  6 | Clock         | LRU |
|  8 |  8 | Random        | Aging |
|  9 | 10 | FIFO          | NRU |
| 10 | 12 | Second chance | LRU |
| 11 | 10 | Clock         | Aging |
| 12 | 12 | Random        | NFU |
| 13 | 12 | FIFO          | LRU |
| 14 | 10 | Second chance | Aging |
| 15 | 12 | Clock         | NFU |
| 16 |  5 | FIFO          | Working set |
| 17 |  5 | Second chance | Working set |
| 18 | 10 | Random        | NRU |
| 19 |  5 | Clock         | Working set |
| 20 |  5 | Random        | Working set |
| 21 | 14 | FIFO          | Aging |
| 22 | 16 | Second chance | NFU |
| 23 | 14 | Clock         | NRU |
| 24 | 16 | Random        | LRU |
| 25 | 16 | FIFO          | NFU |
| 26 | 14 | Second chance | NRU |
| 27 | 16 | Clock         | LRU |
| 28 | 14 | Random        | Aging |
| 29 | 18 | FIFO          | NRU |
| 30 | 18 | Second chance | LRU |

## Дополнительная литература
1. Э. Таненбаум, Х. Бос. Современные операционные системы. 4-е издание. СПб.: Питер, 2015, 2019. (Есть в библиотеке ГУАП) - Глава 3, "Управление памятью"
2. [Wikipedia: Page replacement algorithm](https://en.wikipedia.org/wiki/Page_replacement_algorithm)
