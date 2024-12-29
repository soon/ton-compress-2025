import contextlib
import sys
import base64
import tempfile
import subprocess
import os

import sys
import base64
import lz4.frame

import gzip
import bz2
import pyzstd

@contextlib.contextmanager
def new_cd(x):
    d = os.getcwd()

    # This could raise an exception, but it's probably
    # best to let it propagate and let the caller
    # deal with it, since they requested x
    os.chdir(x)

    try:
        yield

    finally:
        # This could also raise an exception, but you *really*
        # aren't equipped to figure out what went wrong if the
        # old working directory can't be restored.
        os.chdir(d)

def compress_lz4(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)
    # Compress the data with lz4
    compressed_data = lz4.frame.compress(decoded_data, compression_level=lz4.frame.COMPRESSIONLEVEL_MAX)
    # Encode the compressed data back to base64
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_lz4(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)
    # Decompress the data with lz4
    decompressed_data = lz4.frame.decompress(decoded_data)
    # Encode the decompressed data back to base64
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()

def compress_zpaq(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)

    # Use a temporary directory
    with tempfile.TemporaryDirectory() as tempdir:
        with new_cd(tempdir):
          input_file_path = "i"
          archive_path = "archive.zpaq"

          # Write the decoded data to a temporary file
          with open(input_file_path, "wb") as input_file:
              input_file.write(decoded_data)

          # Run zpaq to add the file to an archive
          subprocess.run(["zpaq", "a", archive_path, input_file_path, "-m4"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

          # Read the compressed archive data
          with open(archive_path, "rb") as archive_file:
              compressed_data = archive_file.read()

    # Encode the compressed data back to base64
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_zpaq(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)

    # Use a temporary directory
    with tempfile.TemporaryDirectory() as tempdir:
        with new_cd(tempdir):
          archive_path = "archive.zpaq"

          # Write the compressed archive data to a temporary file
          with open(archive_path, "wb") as archive_file:
              archive_file.write(decoded_data)

          # Run zpaq to extract the archive
          subprocess.run(["zpaq", "x", archive_path], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

          output_file_path = 'i'

          # Read the decompressed data from the output file
          with open(output_file_path, "rb") as output_file:
              decompressed_data = output_file.read()

    # Encode the decompressed data back to base64
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()


def compress_gzip(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)

    # Use a temporary directory
    with tempfile.TemporaryDirectory() as tempdir:
        input_file_path = os.path.join(tempdir, "input_file.gz")

        # Write the compressed data to a temporary file using gzip
        with gzip.open(input_file_path, "wb") as gzip_file:
            gzip_file.write(decoded_data)

        # Read the compressed data from the file
        with open(input_file_path, "rb") as compressed_file:
            compressed_data = compressed_file.read()

    # Encode the compressed data back to base64
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_gzip(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)

    # Use a temporary directory
    with tempfile.TemporaryDirectory() as tempdir:
        input_file_path = os.path.join(tempdir, "input_file.gz")
        output_file_path = os.path.join(tempdir, "output_file")

        # Write the compressed data to a temporary file
        with open(input_file_path, "wb") as compressed_file:
            compressed_file.write(decoded_data)

        # Decompress the data using gzip
        with gzip.open(input_file_path, "rb") as gzip_file:
            decompressed_data = gzip_file.read()

    # Encode the decompressed data back to base64
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()



def compress_7z(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)

    # Use a temporary directory
    with tempfile.TemporaryDirectory() as tempdir:
        input_file_path = os.path.join(tempdir, "input_file")
        archive_path = os.path.join(tempdir, "archive.7z")

        # Write the decoded data to a temporary file
        with open(input_file_path, "wb") as input_file:
            input_file.write(decoded_data)

        # Run 7z to create a compressed archive
        subprocess.run(["7z", "a", archive_path, input_file_path], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        # Read the compressed archive data
        with open(archive_path, "rb") as archive_file:
            compressed_data = archive_file.read()

    # Encode the compressed data back to base64
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_7z(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)

    # Use a temporary directory
    with tempfile.TemporaryDirectory() as tempdir:
        archive_path = os.path.join(tempdir, "archive.7z")
        output_dir = os.path.join(tempdir, "output")
        os.makedirs(output_dir, exist_ok=True)

        # Write the compressed archive data to a temporary file
        with open(archive_path, "wb") as archive_file:
            archive_file.write(decoded_data)

        # Run 7z to extract the archive
        subprocess.run(["7z", "x", archive_path, f"-o{output_dir}"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        # Assuming a single file in the output directory
        output_files = os.listdir(output_dir)
        if len(output_files) != 1:
            raise RuntimeError("Unexpected number of files in archive")

        output_file_path = os.path.join(output_dir, output_files[0])

        # Read the decompressed data from the output file
        with open(output_file_path, "rb") as output_file:
            decompressed_data = output_file.read()

    # Encode the decompressed data back to base64
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()


def compress_combined(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)
    decoded_data = decoded_data.replace(b"\x9e\xa7!.G\x00o\x00\xaa\x89\xd2'3sh\xb1", b'\xa1\x94\xc7\x82')

    compressed_data = lz4.frame.compress(decoded_data, compression_level=lz4.frame.COMPRESSIONLEVEL_MAX)
    # Encode the compressed data back to base64
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_combined(data):
    # Decode the base64 input
    decoded_data = base64.b64decode(data)
    # Decompress the data with lz4
    decompressed_data = lz4.frame.decompress(decoded_data)
    decompressed_data = decompressed_data.replace(b'\xa1\x94\xc7\x82', b"\x9e\xa7!.G\x00o\x00\xaa\x89\xd2'3sh\xb1")
    # Encode the decompressed data back to base64
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()


def compress_bz2(data):
    decoded_data = base64.b64decode(data)
    compressed_data = bz2.compress(decoded_data)
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_bz2(data):
    decoded_data = base64.b64decode(data)
    decompressed_data = bz2.decompress(decoded_data)
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()


def compress_zstd(data):
    decoded_data = base64.b64decode(data)
    compressed_data = pyzstd.compress(decoded_data, 10)
    encoded_compressed_data = base64.b64encode(compressed_data)
    return encoded_compressed_data.decode()

def decompress_zstd(data):
    decoded_data = base64.b64decode(data)
    decompressed_data = pyzstd.decompress(decoded_data)
    encoded_decompressed_data = base64.b64encode(decompressed_data)
    return encoded_decompressed_data.decode()



def compress(data):
    if MODE == "lz4":
        return compress_lz4(data)
    if MODE == "zpaq":
        return compress_zpaq(data)
    if MODE == "gzip":
      return compress_gzip(data)
    if MODE == "7z":
      return compress_7z(data)
    if MODE == "combined":
      return compress_combined(data)
    if MODE == "bz2":
      return compress_bz2(data)
    if MODE == "zstd":
      return compress_zstd(data)
    raise Exception(MODE)


def decompress(data):
    if MODE == "lz4":
        return decompress_lz4(data)
    if MODE == "zpaq":
        return decompress_zpaq(data)
    if MODE == "gzip":
      return decompress_gzip(data)
    if MODE == "7z":
      return decompress_7z(data)
    if MODE == "combined":
      return decompress_combined(data)
    if MODE == "bz2":
      return decompress_bz2(data)
    if MODE == "zstd":
      return decompress_zstd(data)
    raise Exception(MODE)


MODE = 'zpaq'
# MODE = 'lz4'
# MODE = 'bz2'
# MODE = 'zstd'
# MODE = '7z'

def main():
    # Read input lines
    mode = input().strip()
    input_data = input().strip()

    if mode == "compress":
        print(compress(input_data))
    elif mode == "decompress":
        print(decompress(input_data))
    else:
        print("Invalid mode. Please use 'compress' or 'decompress'.", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
