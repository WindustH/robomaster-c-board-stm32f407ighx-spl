import re
from typing import Dict, Optional


class ElfMapParser:
    def __init__(self, elf_file: Optional[str] = None, map_file: Optional[str] = None):
        self.elf_file = elf_file
        self.map_file = map_file
        self.symbols = {}

    def parse_map_file(self) -> Dict[str, int]:
        """Parse map file to extract symbol addresses"""
        if not self.map_file:
            return {}

        try:
            with open(self.map_file, 'r') as f:
                content = f.read()

            # Pattern to match symbol addresses
            pattern = r'0x([0-9a-fA-F]+)\s+(\w+)'
            matches = re.findall(pattern, content)

            for addr_str, symbol in matches:
                self.symbols[symbol] = int(addr_str, 16)

        except Exception as e:
            print(f"Error parsing map file: {e}")

        return self.symbols

    def get_symbol_address(self, symbol_name: str) -> Optional[int]:
        """Get address of a symbol"""
        return self.symbols.get(symbol_name)