import pandas as pd, openpyxl

#reading content of main file
df  = pd.read_excel('SpeciesSheet.xlsx')
count = df[['ImagesCount']].values
pics = []
for num in count:
    pics.append(num[0])

names = df[['SpeciesName']].values
weed = []
for name in names:
    weed.append(name[0])

#reading weedlist in secondary file
file = "ImagesSheet.xlsx"
rb  = pd.read_excel(file)
species = rb[['SpeciesName']].values

#saving the file after updates
wb = openpyxl.load_workbook(file) 
sheet = wb.active
i = 0
for sp in species:
    idx = weed.index(sp[0])
    cnt = pics[idx]
    sheet.cell(row=i+2, column=2).value = cnt
    i = i+1

wb.save(file)