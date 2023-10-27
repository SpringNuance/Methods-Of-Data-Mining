# Extract feature names from birdtable.txt, ignoring the index numbers
cut -d ' ' -f 2- birdtable.txt > feature_names.txt

# Check each feature in feature_names.txt against birdrules.txt.names
while IFS= read -r feature; do
  if ! grep -q "$feature" birdrules.txt.names; then
    echo "$feature"
  fi
done < feature_names.txt > irrelevant_features.txt
