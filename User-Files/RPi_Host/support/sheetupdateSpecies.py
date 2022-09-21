import pandas as pd
import openpyxl
from pathlib import Path

# define the excel sheet files
support_dir = Path("support")
species_file = support_dir / "SpeciesSheet.xlsx"
images_file = support_dir / "ImagesSheet.xlsx"

# opening the main sheet in read mode
species_df = pd.read_excel(species_file)

# getting row counts from main sheet
row_numbers = species_df[['RowNos']].values
row_list = []
for num in row_numbers:
    row_list.append(num[0])

# getting species names from main sheet
species = species_df[['SpeciesName']].values
species_list = []
for sp in species:
    species_list.append(sp[0])

# opening the secondary sheet in write mode
workbook = openpyxl.load_workbook(images_file)
sheet = workbook.active

# scrub the sheet for new incoming data
sheet.delete_cols(1, 2)
sheet["A1"] = "SpeciesName"
sheet["B1"] = "ImagesCount"

# add new data to the sheet
s = 0
for rows in row_list:
    array_1 = rows.split(',')
    species_row = []
    for element in array_1:
        if '-' in element:
            array_2 = [int(e) for e in element.split('-')]
            for val in range(array_2[0], array_2[1]+1):
                species_row.append(val)
        else:
            species_row.append(int(element))

    for val in species_row:
        nam = str(species[s])[2:-2]
        sheet.cell(row=val+1, column=1).value = nam
    s += 1

workbook.save(images_file)
