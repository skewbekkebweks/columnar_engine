# columnar_engine

Колоночный аналитический движок на C++, реализующий 43 запроса [ClickBench](https://benchmark.clickhouse.com/).

## Формат хранения (.skewdb)

Бинарный колоночный формат с разбивкой на row groups. Каждая колонка хранится и сжимается LZ4 независимо от остальных. Заголовок каждой колонки: `[uncompressed_size: 8B][compressed_size: 8B][data]`. Метаданные (схема, смещения колонок, количество строк) хранятся в конце файла.

Поддерживаемые типы: `int64`, `string`, `timestamp`, `date`.

## Сборка

```bash
mkdir build
cd build
cmake ..
make
```

Все зависимости подтягиваются автоматически через FetchContent.

## Использование

**Конвертация CSV → .skewdb:**

```bash
./build/bin/csv_to_columnar \
  --input data/hits.csv \
  --schema data/hits_schema.csv \
  --output data/hits.skewdb
```

Формат файла схемы — CSV с двумя колонками `name,type`:

```
WatchID,int64
Title,string
EventTime,timestamp
EventDate,date
```

**Запуск запросов:**

```bash
# все 43 запроса
./build/bin/run_queries data/hits.skewdb all

# один запрос
./build/bin/run_queries data/hits.skewdb 07
```

**Конвертация .skewdb → CSV:**

```bash
./build/bin/columnar_to_csv --input data/hits.skewdb --output out.csv
```

## Тесты

```bash
cd build && make test
./build/tests/test
```

## Устройство файла .skewdb

```
┌─────────────────────────────────────────────────────┐
│  Row Group 0                                        │
│    col 0:  [ uncompressed_size 8B ]                 │
│            [ compressed_size   8B ]                 │
│            [ LZ4 data            ]                  │
│    col 1:  [ uncompressed_size 8B ]                 │
│            [ compressed_size   8B ]                 │
│            [ LZ4 data            ]                  │
│    ...                                              │
│  Row Group 1                                        │
│    ...                                              │
├─────────────────────────────────────────────────────┤
│  Metadata (для каждого row group):                  │
│    [ rg_offset 8B ][ row_count 8B ]                 │
│    [ col_0_offset 8B ] ... [ col_N_offset 8B ]      │
├─────────────────────────────────────────────────────┤
│  Schema (для каждой колонки):                       │
│    [ name: null-terminated string ][ type: 8B ]     │
├─────────────────────────────────────────────────────┤
│  Footer:                                            │
│    [ schema_offset 8B ]                             │
│    [ columns_count 8B ]                             │
│    [ row_groups_count 8B ]                          │
└─────────────────────────────────────────────────────┘
```

Файл читается с конца: из footer'а берётся `schema_offset` → читается схема → вычисляется и читается блок метаданных → по `col_offset` из метаданных происходит random access к нужным колонкам. Это позволяет читать только те колонки, которые нужны запросу, не трогая остальные.

## Результаты

Измерения на наборе данных 803 МБ (1М строк, ClickBench hits_sample).

| Метрика | Значение |
|---|---|
| Размер после сжатия | 124 МБ (6.5x) |
| CSV → .skewdb | ~13s |
| .skewdb → CSV | ~12.7s |
| Все 43 запроса | ~12s |
