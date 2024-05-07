import pandas as pd
import openpyxl
from pathlib import Path

# define the excel sheet files
support_dir = Path("support")
species_file = support_dir / "SpeciesSheet.xlsx"
images_file = support_dir / "ImagesSheet.xlsx"

# reading content of main file
species_df = pd.read_excel(species_file)
count = species_df[['ImagesCount']].values
pics = []
for num in count:
    pics.append(num[0])

names = species_df[['SpeciesName']].values
weed = []
for name in names:
    weed.append(name[0])

# reading weedlist in secondary file
image_sheet = pd.read_excel(images_file)
species = image_sheet[['SpeciesName']].values

# saving the file after updates
workbook = openpyxl.load_workbook(images_file)
sheet = workbook.active
i = 0
for sp in species:
    if not pd.isna(sp):
        idx = weed.index(sp[0])
        cnt = pics[idx]
        sheet.cell(row=i + 2, column=2).value = cnt
    i = i + 1

workbook.save(images_file)
