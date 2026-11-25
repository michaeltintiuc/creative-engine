"""Embed a compiled Creative Engine resource archive in a portable C++ array."""

import sys
from pathlib import Path
from zipfile import ZipFile


def main():
    archive_path, output_dir = map(Path, sys.argv[1:])
    output_dir.mkdir(parents=True, exist_ok=True)
    with ZipFile(archive_path) as archive:
        header = archive.read("Resources.h")
        resources = archive.read("Resources.bin")

    (output_dir / "Resources.h").write_bytes(header)
    with (output_dir / "Resources.cpp").open("w", encoding="utf-8") as output:
        output.write('#include "BTypes.h"\nalignas(16) TUint8 gCreativeEngineResources[] = {\n')
        for offset in range(0, len(resources), 32):
            output.write(",".join(map(str, resources[offset:offset + 32])) + ",\n")
        output.write("};\n")


if __name__ == "__main__":
    main()
