import os

rsp_path = r'C:/Users/fjmn2/Dev/neon-district-sandbox/Intermediate/Build/Win64/x64/NeonDistrict/Development/NeonDistrict.exe.rsp'
out_path = r'C:/Users/fjmn2/Dev/neon-district-sandbox/Intermediate/Build/Win64/x64/NeonDistrict/Development/NeonDistrict_abs.rsp'

engine_bin = 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64'
proj_bin = 'C:/Users/fjmn2/Dev/neon-district-sandbox/Binaries/Win64'

with open(rsp_path) as f:
    rsp = f.read()

tokens = rsp.split()
out = []
for tok in tokens:
    if tok.startswith('..\\Intermediate'):
        # Engine-relative path (.. from Engine/Binaries/Win64 -> Engine/Intermediate)
        rel = tok[3:]  # strip '..\\'
        out.append(engine_bin + '/' + rel.replace('\\', '/'))
    else:
        out.append(tok)

with open(out_path, 'w') as f:
    f.write(' '.join(out))

print('written', out_path)
print('sample:', out[5] if len(out) > 5 else '')
