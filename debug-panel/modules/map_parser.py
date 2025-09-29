import re
from typing import Dict, Optional


class MapParser:
    def __init__(self, map_file: Optional[str] = None):
        self.map_file = map_file
        self.symbols = {}

    def parse_map_file(self) -> Dict[str, int]:
        """Parse map file to extract symbol addresses"""
        if not self.map_file:
            return {}

        try:
            with open(self.map_file, 'r') as f:
                lines = f.readlines()

            # Parse the map file line by line
            for i, line in enumerate(lines):
                # Look for lines that contain symbol names (they start with a dot)
                if line.strip().startswith('.'):
                    parts = line.strip().split()
                    if len(parts) >= 1:
                        symbol_name = parts[0]
                        # Remove all section prefixes (like .bss., .data., .text.)
                        while '.' in symbol_name:
                            symbol_name = symbol_name.split('.', 1)[1]

                        # Check if next line contains the address
                        if i + 1 < len(lines):
                            next_line = lines[i + 1].strip()
                            # Look for hex addresses starting with 0x
                            addr_match = re.search(r'0x([0-9a-fA-F]+)', next_line)
                            if addr_match:
                                address = int(addr_match.group(1), 16)
                                self.symbols[symbol_name] = address

        except Exception as e:
            print(f"Error parsing map file: {e}")

        return self.symbols

    def get_symbol_address(self, symbol_name: str) -> Optional[int]:
        """Get address of a symbol"""
        return self.symbols.get(symbol_name)