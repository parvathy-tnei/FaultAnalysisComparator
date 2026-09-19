# Fault Analysis Comparator

Fault Analysis Comparator is a Qt/C++ desktop application for comparing IPSA fault-analysis results exported as CSV files.

The application provides a simple workflow for loading multiple fault-analysis result files, selecting the data to compare, viewing the results in a comparison table, reviewing fault-analysis configuration settings, and exporting the comparison results as a PDF report.

The tool is designed to compare fault-analysis results without modifying the original CSV data.

---

## Overview

Fault Analysis Comparator allows users to compare fault-analysis results generated from IPSA/PyIPSA workflows.

A typical workflow is:

1. Load one or more fault-analysis CSV files.
2. Review the fault-analysis configuration/settings associated with each file.
3. Select the CSV columns that should be displayed.
4. View the selected results in a comparison table.
5. Reorder comparison columns when required.
6. Export the comparison results and configured fault settings as a PDF report.

The application treats the imported CSV data as comparison input. The values are read and displayed without modifying the original source data.

---

## Features

### Fault Analysis CSV Loading

* Load multiple fault-analysis CSV files.
* Support CSV files generated from IPSA/PyIPSA fault-analysis workflows.
* Display loaded files in a common **Files to Compare** area.
* Enable or disable individual files for comparison.
* Clear all loaded files safely.
* Continue to use the comparison functionality when only one CSV file is loaded.

### CSV Parsing

The application includes a dedicated CSV parser for reading fault-analysis result files.

The parser:

* Reads CSV headers.
* Handles comma-delimited data.
* Supports semicolon-delimited headers where required.
* Removes a UTF-8 BOM when present.
* Preserves imported values for comparison.
* Separates file parsing from the comparison UI.

The CSV data is not recalculated or altered by the comparator.

---

## Fault Analysis Results

The comparator can display fault-analysis result fields such as:

* Name
* AC Magnitude (kA)
* DC Magnitude (kA)
* DC %
* Second Harmonic (kA)
* DC X/R (Driving Point)
* DC Thevenin X/R
* Red Phase Magnitude
* Red Phase Angle
* Yellow Phase Magnitude
* Yellow Phase Angle
* Blue Phase Magnitude
* Blue Phase Angle
* Positive Phase Magnitude
* Positive Phase Angle
* Negative Phase Magnitude
* Negative Phase Angle
* Zero Phase Magnitude
* Zero Phase Angle

The available columns depend on the contents of the loaded CSV files.

---

## Comparison Table

The **Comparison Table** displays the selected fault-analysis results from the loaded files.

Users can:

* Select which CSV columns are displayed.
* Compare values from multiple fault-analysis files.
* View results from different files within the same comparison table.
* Reorder columns using drag and drop.
* Preserve the selected column arrangement for reporting.

The comparison table is intended to provide a direct side-by-side view of the selected fault-analysis results.

### Column Selection

A column-selection panel allows the user to choose the fields that should appear in the comparison table.

Only the selected columns are displayed.

This keeps the comparison focused on the fault-analysis parameters relevant to the user.

### Column Reordering

Comparison-table columns can be rearranged using drag and drop.

The final column arrangement is used when generating the comparison report so that the exported report follows the user's chosen layout.

---

## Fault Analysis Configuration

The application provides a separate view for reviewing fault-analysis settings/configuration associated with the loaded files.

This allows users to compare not only the resulting fault levels but also the configuration used for the fault-analysis cases.

Depending on the available information, configuration data can include settings such as:

* Calculate type
* Fault type
* Fault result type
* Fault resistance
* Fault reactance
* Fault time

The configuration is displayed separately from the main comparison table.

This prevents the result data and configuration information from being mixed together while still allowing both to be included in the exported report.

---

## Supported Fault Types

The application is intended to support comparison of different IPSA fault-analysis cases.

Examples include:

* Line-to-ground (LG)
* Line-to-line (LL)
* Double-line-to-ground (LLG)
* Three-phase / three-line faults (LLL)

The comparator does not modify the fault-analysis settings contained in the source data.

---

## Fault Analysis Data Generation

The CSV files used by the comparator can be generated using IPSA/PyIPSA scripting.

A typical PyIPSA workflow is:

```text
GetScriptInterface()
        |
        v
GetNetwork()
        |
        v
GetAnalysisFL()
        |
        v
Configure fault-analysis parameters
        |
        v
Run fault analysis
        |
        v
GetResultsTableText(BusbarFL)
        |
        v
Save results as CSV
```

Typical fault-analysis configuration can include:

```text
* Calculate type
* Fault type
* Fault result type
* Fault resistance
* Fault reactance
* Fault time
```

The resulting CSV files can then be loaded into Fault Analysis Comparator for comparison.

---

## Application Workflow

### 1. Launch the application

The application provides access to the comparison tools through the common comparator launcher.

The launcher provides:

```text
Select Comparison Tool

Transient Analysis Comparison
Fault Analysis Comparison
```

The Fault Analysis Comparison option opens the fault-analysis comparison interface.

---

### 2. Load CSV files

Use:

```text
File -> Add CSV File(s)
```

Select one or more fault-analysis CSV files.

The loaded files are displayed in the **Files to Compare** area.

Individual files can be closed if not required for the comparison.

---

### 3. Select comparison columns

Use the column-selection area to choose the fault-analysis fields that should appear in the comparison table.

For example:

```text
Name
AC Mag. (kA)
DC Mag. (kA)
DC %
Positive Phase Mag.
Positive Phase Angle
Negative Phase Mag.
Negative Phase Angle
Zero Phase Mag.
Zero Phase Angle
```

Only selected columns are displayed in the comparison table.

---

### 4. Review the comparison table

The selected data is displayed in:

```text
Comparison Table
```

The table allows the user to compare the selected fields across the loaded CSV files.

Columns can be rearranged using drag and drop.

---

### 5. Review fault settings

Open the fault-analysis configuration/settings view to review the fault settings associated with the loaded files.

This provides a separate comparison of the configuration used for each fault-analysis case.

---

### 6. Export the comparison

The comparison can be exported as a PDF report.

The report can contain:

* Fault-analysis configuration/settings
* Comparison-table data
* The final column arrangement selected by the user

---

## Export

The application supports exporting fault-analysis comparison results as a PDF report.

The exported report is intended to provide a professional representation of the comparison performed in the application.

The report can include both:

### Fault Analysis Configuration

The configured fault-analysis settings for each loaded file.

### Comparison Results

The selected and reordered comparison-table data.

The report therefore contains both the conditions used for the fault analysis and the resulting values being compared.

---

## User Interface

The application uses a Qt Widgets-based interface.

The interface is designed around a simple comparison workflow:

```text
+-------------------------------------------------------+
|                 Fault Analysis Comparator             |
+-------------------------------------------------------+
| Files to Compare                                      |
|                                                       |
|  [File 1]                                             |
|  [File 2]                                             |
|  [File 3]                                             |
|                                                       |
+----------------------+--------------------------------+
| Column Selection     | Comparison Table               |
|                      |                                |
| [x] Name             | Name | AC Mag | DC Mag | ... |
| [x] AC Mag.          |--------------------------------|
| [ ] DC Mag.          | ...                            |
| [x] DC %
```
