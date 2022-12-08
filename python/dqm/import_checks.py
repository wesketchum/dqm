import importlib
import sys
d = {}
for module in ['kafka', 'msgpack', 'scipy', 'numpy']:
    try:
        d[module] = importlib.import_module(module)
    except ModuleNotFoundError:
        print(f'''Module {module} is not installed. Run "pip install {module if module != 'kafka' else 'kafka-python'}"''')
        sys.exit()
kafka = d['kafka']
msgpack = d['msgpack']
scipy = d['scipy']
numpy = d['numpy']
