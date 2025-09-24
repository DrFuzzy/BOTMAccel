#!/bin/bash

# Create the data directory if it doesn't exist
mkdir -p ./data

# Define the URL
URL="https://figshare.com/ndownloader/files/16442771"
FOLDER_NAME="ushant_ais"

# Download the file, letting wget pick the filename
echo "Downloading dataset..."
wget --content-disposition \
     --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0 Safari/537.36" \
     --referer="https://figshare.com" \
     -P ./data \
     "$URL"

# Find the most recent file in ./data (the one we just downloaded)
ZIP_FILE=$(ls -t ./data/*.zip | head -n 1)

# Create destination dir
DEST_DIR="./data/input/$FOLDER_NAME"
mkdir -p "$DEST_DIR"

# Extract directly
echo "Extracting dataset into $DEST_DIR..."
unzip -q "$ZIP_FILE" -d "$DEST_DIR"

# Remove the zip file
echo "Cleaning up..."
rm "$ZIP_FILE"

echo "Done. Dataset is available in $DEST_DIR"
