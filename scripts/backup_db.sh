#!/bin/bash

# UmbraEternum Database Backup Script
# Version: 1.3.0

echo "UmbraEternum Database Backup"
echo "============================"
echo ""

# Configuration
DB_NAME="umbra_eternum"
DB_USER="root"
DB_PASS=""
BACKUP_DIR="../backups"
DATE=$(date +"%Y%m%d_%H%M%S")
BACKUP_FILE="$BACKUP_DIR/umbra_db_$DATE.sql"

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Perform backup
echo "Backing up database '$DB_NAME'..."
mysqldump -u "$DB_USER" -p"$DB_PASS" "$DB_NAME" > "$BACKUP_FILE"

if [ $? -eq 0 ]; then
  echo "Backup successful: $BACKUP_FILE"
  
  # Compress backup
  gzip "$BACKUP_FILE"
  echo "Compressed: ${BACKUP_FILE}.gz"
  
  # Delete old backups (keep last 7 days)
  find "$BACKUP_DIR" -name "umbra_db_*.sql.gz" -mtime +7 -delete
  echo "Old backups cleaned up (kept last 7 days)"
else
  echo "Backup failed!"
  exit 1
fi

echo ""
echo "Backup complete!"

