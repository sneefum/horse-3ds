from PIL import Image
import sys
from pathlib import Path
import pyperclip

def image_to_c_array(image_path):
    # Load image as RGBA
    img = Image.open(image_path).convert("RGBA")
    pixels = img.load()

    colors = []

    width, height = img.size

    # Iterate through every pixel
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]

            # Ignore transparent pixels
            if a > 180:
                colors.append((r, g, b))
            else:
                colors.append((0, 0, 0))

    # Build C array string
    lines = [f"unsigned char horse[30000] = {{"]

    for r, g, b in colors:
        lines.append(f"    {r}, {g}, {b},")

    lines.append("};")
    lines.append(f"\n// Total RGB triplets: {len(colors)}")
    lines.append(f"// Total bytes: {len(colors) * 3}")

    output = "\n".join(lines)

    # Print to terminal
    print(output)

    # Copy to clipboard
    pyperclip.copy(output)

    print("\nCopied to clipboard.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <image>")
        sys.exit(1)

    image_path = Path(sys.argv[1])

    if not image_path.exists():
        print("Image file not found.")
        sys.exit(1)

    image_to_c_array(image_path)
