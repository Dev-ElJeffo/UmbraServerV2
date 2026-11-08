-- Catálogo de partes modulares (hair/head). Idempotente.
SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS player_appearance_parts (
  appearance_part_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  part_type ENUM('body', 'hair', 'head') NOT NULL,
  part_id INT UNSIGNED NOT NULL DEFAULT 0,
  mesh_path VARCHAR(512) DEFAULT NULL,
  attach_socket VARCHAR(64) NOT NULL DEFAULT 'head',
  is_enabled TINYINT(1) NOT NULL DEFAULT 1,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (appearance_part_id),
  UNIQUE KEY uq_part_type_id (part_type, part_id),
  INDEX idx_part_type_enabled (part_type, is_enabled)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- part_id=0: sem mesh extra (default)
INSERT INTO player_appearance_parts (part_type, part_id, mesh_path, attach_socket, is_enabled)
VALUES
  ('hair', 0, NULL, 'head', 1),
  ('head', 0, NULL, 'head', 1)
ON DUPLICATE KEY UPDATE
  mesh_path = VALUES(mesh_path),
  attach_socket = VALUES(attach_socket),
  is_enabled = VALUES(is_enabled);
