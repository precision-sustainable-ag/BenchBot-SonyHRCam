import pandas as pd, openpyxl
 
# opening the main sheet in read mode
df  = pd.read_excel('SpeciesSheet.xlsx')
 
# getting row counts from main sheet
colvalues = df[['RowNos']].values
rarr = []
for num in colvalues:
   rarr.append(num[0])
 
# getting species names from main sheet
species = df[['SpeciesName']].values
sarr = []
for num in species:
   sarr.append(num[0])
 
# opening the secondary sheet in write mode
wb = openpyxl.load_workbook('ImagesSheet.xlsx')
sheet = wb.active
 
s = 0
for rownums in rarr:
   arr1 = rownums.split(',')
   species_row = []
   for elmnt in arr1:
      if '-' in elmnt:
         arr2 = [int(e) for e in elmnt.split('-')]
         for val in range(arr2[0], arr2[1]+1):
               species_row.append(val)
      else:
         species_row.append(int(elmnt))

   for val in species_row:
       nam = str(species[s])[2:-2]
       sheet.cell(row = val+1, column = 1).value = nam
   s+= 1
 
wb.save('ImagesSheet.xlsx')
