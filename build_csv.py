import openpyxl, csv, re

wb = openpyxl.load_workbook('/mnt/user-data/uploads/PRICE-JC8-TRADING.xlsx', data_only=True)
ws = wb['PRICES']

UNIT_MAP = {
    'pc': 'PIECE', 'pcs': 'PIECE', 'set': 'SET', 'roll': 'ROLL', 'box': 'BOX',
    'pack': 'PACK', 'ream': 'REAM', 'bundle': 'BUNDLE', 'ctn': 'BUNDLE',
    '1000pcs': 'BUNDLE', '100 pcs': 'PACK', 'gal': 'GALLON', 'kilo': 'KILO',
}

def norm_unit(u):
    if u is None:
        return None
    if isinstance(u, (int, float)):
        return 'PIECE'
    return UNIT_MAP.get(str(u).strip().lower(), 'OTHER')

rows = []
category = ''
header_unit = None
for r in ws.iter_rows(min_row=1, max_row=314, values_only=True):
    name, price, unit = r[0], r[1], r[2]
    if name is None and price is None:
        category = ''
        header_unit = None
        continue
    if name is None:
        continue
    name = re.sub(r'\s+', ' ', str(name)).strip()
    if not isinstance(price, (int, float)):
        if price is None and name.lower() != 'individually wrapped':
            category = name
            header_unit = norm_unit(unit)
        continue
    u = norm_unit(unit) or header_unit or 'PIECE'
    rows.append((category, name, u, price))

with open('data/products.csv', 'w', newline='', encoding='utf-8') as f:
    w = csv.writer(f, lineterminator='\n')
    w.writerow(['code', 'name', 'category', 'unit', 'price'])
    for i, (cat, name, u, p) in enumerate(rows, 1):
        w.writerow([f'JC8-{i:04d}', name, cat, u, f'{p:.2f}'])
print(len(rows))
