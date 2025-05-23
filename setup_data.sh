#!/bin/bash

# Create the data directory if it doesn't exist
mkdir -p ./data

# Define the URL and target zip file name
URL="https://figshare.com/ndownloader/files/16442771"
ZIP_FILE="./data/dataset.zip"
FOLDER_NAME="dataset"

# Download the zip file
echo "Downloading dataset..."
curl -L "$URL" -o "$ZIP_FILE"

# Create a subdirectory named after the zip file (without .zip)
DEST_DIR="./data/$FOLDER_NAME"
mkdir -p "$DEST_DIR"

# Extract the zip file into the subdirectory
echo "Extracting dataset into $DEST_DIR..."
unzip -q "$ZIP_FILE" -d "$DEST_DIR"

# Remove the zip file
echo "Cleaning up..."
rm "$ZIP_FILE"

echo "Done. Dataset is available in $DEST_DIR"
