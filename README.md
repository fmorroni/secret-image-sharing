# Secret Image Sharing Tool

This tool enables you to distribute a secret BMP image by embedding hidden shadow images into carrier BMP files using a combination of secret sharing and steganography. Later, the original secret can be recovered from a subset of these carrier images. The program supports flexible configuration options, including customizable directories, seeding, and more.

---

## 📦 Features

- Split a BMP image into multiple carrier images
- Recover a secret BMP image from a threshold number of carrier images
- Seeding for encription
- Header inspection
- Directory customization for input and output

---

## 🔧 Compilation

This project includes a `Makefile` with the following targets:

| Target  | Description                                 |
|---------|---------------------------------------------|
| `all`   | Builds the release version into `./bin/app` |
| `clean` | Removes compiled binaries and objects       |
| `lint`  | Runs the linter on the source code          |
| `debug` | Cleans, then builds with debug symbols      |

### To build the program:

```
make all
```

### For debugging:

```
make debug
```

### Clean the build artifacts:

```
make clean
```

---

## 🚀 Usage

```
./<executable_name> (-r | -d) -s FILE -k NUM [options]
```

### Required Arguments

- `-d`, `--distribute`  
  Distribute a secret into shadow images. **Mutually exclusive with `-r`.**

- `-r`, `--recover`  
  Recover a secret from shadow images. **Mutually exclusive with `-d`.**

- `-s FILE`, `--secret FILE`  
  - With `-d`: BMP image file to hide/distribute  
  - With `-r`: Output file for the recovered secret

- `-k NUM`, `--min-shadows NUM`  
  Minimum number of shadows required to reconstruct the secret.  
  **Must satisfy: 2 ≤ NUM ≤ 255**

### Optional Arguments

- `-n NUM`, `--tot-shadows NUM`  
  Total number of shadows to create (only with `-d`)

- `-D DIR`, `--dir DIR`  
  Directory to read shadow images from  
  *(Default: current working directory)*

- `-O DIR`, `--dir-out DIR`  
  Directory to write shadow images to (only with `-d`)  
  *(default: the value assigned to `--dir`)*

- `-S NUM`, `--seed NUM`  
  Seed for permutation matrix  
  *(Default: 0 when distributing; detect from header when recovering)*

- `-p`, `--print-header`  
  Print the BMP header of the input image (for inspection/debugging)

- `-h`, `--help`  
  Show help message and exit

---

## 📚 Example Commands

### Distribute a secret image into 5 carrier images found in the cwd with a threshold of 3:

```
./secretshare -d -s secret.bmp -k 3 -n 5 -O ./shadows
```

### Recover the secret image from shadows in a specified directory:

```
./secretshare -r -s recovered.bmp -k 3 -D ./shadows
```

### Print header of a BMP file:

```
./secretshare -p -s secret.bmp
```

---

## 📝 License

This project was developed for educational purposes as part of a university assignment. Feel free to use or adapt it under the terms of your institution’s policies.

The method implemented is based on the paper *"An Efficient Secret Image Sharing Scheme"* by Kuang-Shyr Wu and Tsung-Ming Lo.
