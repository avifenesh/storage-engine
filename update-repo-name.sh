#!/bin/bash

# Script to update repository name after GitHub rename
# Usage: ./update-repo-name.sh <new-repo-name>

if [ $# -eq 0 ]; then
    echo "Usage: $0 <new-repo-name>"
    echo "Example: $0 kernel-text-search"
    exit 1
fi

NEW_NAME=$1
OLD_NAME="C-refresher"

echo "Updating repository from $OLD_NAME to $NEW_NAME..."

# Update git remote
echo "1. Updating git remote URL..."
git remote set-url origin "https://github.com/avifenesh/$NEW_NAME.git"

# Verify the change
echo "2. New remote URL:"
git remote -v

# Update documentation
echo "3. Updating documentation references..."
sed -i "s/$OLD_NAME/$NEW_NAME/g" docs/LEARNING_PROGRESS.md

# Test the connection
echo "4. Testing connection to new repository..."
git fetch

echo "✅ Repository name update complete!"
echo ""
echo "Next steps:"
echo "1. Review the changes: git diff"
echo "2. Commit the documentation updates: git add -A && git commit -m 'Update repository name to $NEW_NAME'"
echo "3. Push to the renamed repository: git push"