# File-to-Image Encryptor

A C program that encrypts any file into a BMP image using XOR encryption and color-based encoding. The encrypted data is visually represented as a bitmap image, providing both encryption and steganographic properties.

## Features

- Encrypts any file type into a BMP image
- Uses XOR encryption with a user-provided key
- Converts encrypted bytes into color patterns
- Maintains data integrity through precise color mapping
- Supports both encryption and decryption operations

## Usage

### Compilation

```bash
gcc -o encryptor main.c -lm
```

### Encrypting a File

To encrypt a file into an image:

```bash
./encryptor -i <input_file> <encryption_key>
```

Example:
```bash
./encryptor -i secret.pdf mysecretkey
```
Output will be saved as `encoded.bmp`

### Decrypting a File

To decrypt an encoded image back to the original file:

```bash
./encryptor -o <encoded_image> <encryption_key> <output_file>
```

Example:
```bash
./encryptor -o encoded.bmp mysecretkey recovered_secret.pdf
```

## How It Works

1. **File Processing**
   - Input file is read byte by byte
   - Each byte is encrypted using XOR with the provided key
   - Encrypted bytes are mapped to RGB color values

2. **Image Generation**
   - Creates a square BMP image large enough to store the file
   - Converts encrypted data into color patterns using position-based algorithms
   - Adds gradient padding for incomplete blocks

3. **Color Encoding**
   - Uses three different encoding patterns based on byte position:
     - Position 0: Primary RED encoding
     - Position 1: Primary GREEN encoding
     - Position 2: Primary BLUE encoding

4. **Decryption Process**
   - Reads the color values from the BMP image
   - Extracts the encrypted bytes using position-based color recovery
   - Decrypts the data using the same XOR key
   - Reconstructs the original file

## Technical Details

- Uses Windows BMP format (24-bit color depth)
- Implements proper BMP padding for row alignment
- Handles files of any size (limited by available memory)
- Preserves original file content perfectly
- Uses pure C

## Notes

- The encryption key length affects the encryption strength
- The output image size is determined by input file size
- Gradient padding helps disguise the encrypted data
- BMP format was chosen for its simplicity and lossless nature

## License

This project is available under the MIT License.

## Contributing

Feel free to submit issues and enhancement requests!
