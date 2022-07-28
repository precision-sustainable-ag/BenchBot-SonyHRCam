import pandas as pd
import openpyxl

# reading content of main file
species_df = pd.read_excel('SpeciesSheet.xlsx')
count = species_df[['ImagesCount']].values
pics = []
for num in count:
    pics.append(num[0])

names = species_df[['SpeciesName']].values
weed = []
for name in names:
    weed.append(name[0])

# reading weedlist in secondary file
file = "ImagesSheet.xlsx"
image_sheet = pd.read_excel(file)
species = image_sheet[['SpeciesName']].values

# saving the file after updates
workbook = openpyxl.load_workbook(file)
sheet = workbook.active
i = 0
for sp in species:
    idx = weed.index(sp[0])
    cnt = pics[idx]
    sheet.cell(row=i+2, column=2).value = cnt
    i = i+1

workbook.save(file)
