```php
<?php
// Ensure no output before this line
// Disable error display, log to file
ini_set('display_errors', 0);
ini_set('display_startup_errors', 0);
error_reporting(E_ALL);
ini_set('log_errors', 1);
ini_set('error_log', dirname(__FILE__) . '/php_errors.log'); // Log to script directory

// Start output buffering
ob_start();

// Set content type
header('Content-Type: text/html; charset=utf-8');

// Handle raw JSON request
if (isset($_GET['raw_json'])) {
    header('Content-Type: application/json; charset=utf-8');
    $mapData = [
        "maps" => [
            [
                "name" => "Regenshein",
                "type" => "City (human capital)",
                "global_center" => [5000, -5000],
                "connections" => [
                    ["direction" => "north", "global" => [5000, -4500], "to_map" => "Verband", "to_direction" => "south", "to_global" => [5000, -4500]]
                ]
            ],
            [
                "name" => "Verband",
                "type" => "Trade town",
                "global_center" => [5000, -4000],
                "connections" => [
                    ["direction" => "south", "global" => [5000, -4500], "to_map" => "Regenshein", "to_direction" => "north", "to_global" => [5000, -4500]],
                    ["direction" => "north", "global" => [5000, -3500], "to_map" => "Amarkand", "to_direction" => "south", "to_global" => [5000, -3500]]
                ]
            ],
            [
                "name" => "Amarkand",
                "type" => "Desert with ruins",
                "global_center" => [5000, -3000],
                "connections" => [
                    ["direction" => "south", "global" => [5000, -3500], "to_map" => "Verband", "to_direction" => "north", "to_global" => [5000, -3500]],
                    ["direction" => "west", "global" => [4500, -3000], "to_map" => "Estrada Trifurcada Halperin", "to_direction" => "east", "to_global" => [4500, -3000]],
                    ["direction" => "southwest", "global" => [4500, -3500], "to_map" => "Floresta Cinzenta", "to_direction" => "northeast", "to_global" => [4500, -3500]]
                ]
            ],
            [
                "name" => "Estrada Trifurcada Halperin",
                "type" => "Natural hub/transition area",
                "global_center" => [4000, -3000],
                "connections" => [
                    ["direction" => "east", "global" => [4500, -3000], "to_map" => "Amarkand", "to_direction" => "west", "to_global" => [4500, -3000]],
                    ["direction" => "north", "global" => [4000, -2500], "to_map" => "Deserto Sigmund", "to_direction" => "south", "to_global" => [4000, -2500]],
                    ["direction" => "southwest", "global" => [3500, -3500], "to_map" => "Floresta Cinzenta", "to_direction" => "northeast", "to_global" => [3500, -3500]]
                ]
            ],
            [
                "name" => "Deserto Sigmund",
                "type" => "Large central desert",
                "global_center" => [4000, -2000],
                "connections" => [
                    ["direction" => "south", "global" => [4000, -2500], "to_map" => "Estrada Trifurcada Halperin", "to_direction" => "north", "to_global" => [4000, -2500]],
                    ["direction" => "west", "global" => [3500, -2000], "to_map" => "Sítio Arqueológico Cahill", "to_direction" => "east", "to_global" => [3500, -2000]],
                    ["direction" => "southwest", "global" => [3500, -2500], "to_map" => "Mina Lenfer", "to_direction" => "northeast", "to_global" => [3500, -2500]]
                ]
            ],
            [
                "name" => "Sítio Arqueológico Cahill",
                "type" => "Ruins and excavation site",
                "global_center" => [3000, -2000],
                "connections" => [
                    ["direction" => "east", "global" => [3500, -2000], "to_map" => "Deserto Sigmund", "to_direction" => "west", "to_global" => [3500, -2000]],
                    ["direction" => "northwest", "global" => [2500, -1500], "to_map" => "Basilan", "to_direction" => "southeast", "to_global" => [2500, -1500]],
                    ["direction" => "south", "global" => [3000, -2500], "to_map" => "Floresta Ungor", "to_direction" => "north", "to_global" => [3000, -2500]]
                ]
            ],
            [
                "name" => "Basilan",
                "type" => "Rocky, fortified region",
                "global_center" => [2000, -1000],
                "connections" => [
                    ["direction" => "southeast", "global" => [2500, -1500], "to_map" => "Sítio Arqueológico Cahill", "to_direction" => "northwest", "to_global" => [2500, -1500]]
                ]
            ],
            [
                "name" => "Mina Lenfer",
                "type" => "Dark underground complex",
                "global_center" => [3000, -3000],
                "connections" => [
                    ["direction" => "northeast", "global" => [3500, -2500], "to_map" => "Deserto Sigmund", "to_direction" => "southwest", "to_global" => [3500, -2500]]
                ]
            ],
            [
                "name" => "Floresta Ungor",
                "type" => "Large central forest",
                "global_center" => [3000, -3000],
                "connections" => [
                    ["direction" => "north", "global" => [3000, -2500], "to_map" => "Sítio Arqueológico Cahill", "to_direction" => "south", "to_global" => [3000, -2500]],
                    ["direction" => "east", "global" => [3500, -3000], "to_map" => "Floresta Cinzenta", "to_direction" => "west", "to_global" => [3500, -3000]],
                    ["direction" => "southwest", "global" => [2500, -3500], "to_map" => "Floresta Zeelant", "to_direction" => "northeast", "to_global" => [2500, -3500]],
                    ["direction" => "southeast", "global" => [3500, -3500], "to_map" => "Torre de Giovanni", "to_direction" => "northwest", "to_global" => [3500, -3500]]
                ]
            ],
            [
                "name" => "Floresta Cinzenta",
                "type" => "Dark, misty forest",
                "global_center" => [4000, -3000],
                "connections" => [
                    ["direction" => "west", "global" => [3500, -3000], "to_map" => "Floresta Ungor", "to_direction" => "east", "to_global" => [3500, -3000]],
                    ["direction" => "northeast", "global" => [4500, -2500], "to_map" => "Estrada Trifurcada Halperin", "to_direction" => "southwest", "to_global" => [3500, -3500]],
                    ["direction" => "northeast", "global" => [4500, -2500], "to_map" => "Amarkand", "to_direction" => "southwest", "to_global" => [4500, -3500]],
                    ["direction" => "southeast", "global" => [4500, -3500], "to_map" => "Torre de Giovanni", "to_direction" => "northeast", "to_global" => [4500, -3500]]
                ]
            ],
            [
                "name" => "Floresta Zeelant",
                "type" => "Wet, deep forest",
                "global_center" => [2000, -4000],
                "connections" => [
                    ["direction" => "northeast", "global" => [2500, -3500], "to_map" => "Floresta Ungor", "to_direction" => "southwest", "to_global" => [2500, -3500]]
                ]
            ],
            [
                "name" => "Torre de Giovanni",
                "type" => "Mystic ruined tower",
                "global_center" => [4000, -4000],
                "connections" => [
                    ["direction" => "northwest", "global" => [3500, -3500], "to_map" => "Floresta Ungor", "to_direction" => "southeast", "to_global" => [3500, -3500]],
                    ["direction" => "northeast", "global" => [4500, -3500], "to_map" => "Floresta Cinzenta", "to_direction" => "southeast", "to_global" => [4500, -3500]]
                ]
            ]
        ]
    ];
    echo json_encode($mapData['maps'], JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_QUOT | JSON_HEX_APOS);
    ob_end_flush();
    exit;
}

// Map data
$mapData = [
    "maps" => [
        [
            "name" => "Regenshein",
            "type" => "City (human capital)",
            "global_center" => [5000, -5000],
            "connections" => [
                ["direction" => "north", "global" => [5000, -4500], "to_map" => "Verband", "to_direction" => "south", "to_global" => [5000, -4500]]
            ]
        ],
        [
            "name" => "Verband",
            "type" => "Trade town",
            "global_center" => [5000, -4000],
            "connections" => [
                ["direction" => "south", "global" => [5000, -4500], "to_map" => "Regenshein", "to_direction" => "north", "to_global" => [5000, -4500]],
                ["direction" => "north", "global" => [5000, -3500], "to_map" => "Amarkand", "to_direction" => "south", "to_global" => [5000, -3500]]
            ]
        ],
        [
            "name" => "Amarkand",
            "type" => "Desert with ruins",
            "global_center" => [5000, -3000],
            "connections" => [
                ["direction" => "south", "global" => [5000, -3500], "to_map" => "Verband", "to_direction" => "north", "to_global" => [5000, -3500]],
                ["direction" => "west", "global" => [4500, -3000], "to_map" => "Estrada Trifurcada Halperin", "to_direction" => "east", "to_global" => [4500, -3000]],
                ["direction" => "southwest", "global" => [4500, -3500], "to_map" => "Floresta Cinzenta", "to_direction" => "northeast", "to_global" => [4500, -3500]]
            ]
        ],
        [
            "name" => "Estrada Trifurcada Halperin",
            "type" => "Natural hub/transition area",
            "global_center" => [4000, -3000],
            "connections" => [
                ["direction" => "east", "global" => [4500, -3000], "to_map" => "Amarkand", "to_direction" => "west", "to_global" => [4500, -3000]],
                ["direction" => "north", "global" => [4000, -2500], "to_map" => "Deserto Sigmund", "to_direction" => "south", "to_global" => [4000, -2500]],
                ["direction" => "southwest", "global" => [3500, -3500], "to_map" => "Floresta Cinzenta", "to_direction" => "northeast", "to_global" => [3500, -3500]]
            ]
        ],
        [
            "name" => "Deserto Sigmund",
            "type" => "Large central desert",
            "global_center" => [4000, -2000],
            "connections" => [
                ["direction" => "south", "global" => [4000, -2500], "to_map" => "Estrada Trifurcada Halperin", "to_direction" => "north", "to_global" => [4000, -2500]],
                ["direction" => "west", "global" => [3500, -2000], "to_map" => "Sítio Arqueológico Cahill", "to_direction" => "east", "to_global" => [3500, -2000]],
                ["direction" => "southwest", "global" => [3500, -2500], "to_map" => "Mina Lenfer", "to_direction" => "northeast", "to_global" => [3500, -2500]]
            ]
        ],
        [
            "name" => "Sítio Arqueológico Cahill",
            "type" => "Ruins and excavation site",
            "global_center" => [3000, -2000],
            "connections" => [
                ["direction" => "east", "global" => [3500, -2000], "to_map" => "Deserto Sigmund", "to_direction" => "west", "to_global" => [3500, -2000]],
                ["direction" => "northwest", "global" => [2500, -1500], "to_map" => "Basilan", "to_direction" => "southeast", "to_global" => [2500, -1500]],
                ["direction" => "south", "global" => [3000, -2500], "to_map" => "Floresta Ungor", "to_direction" => "north", "to_global" => [3000, -2500]]
            ]
        ],
        [
            "name" => "Basilan",
            "type" => "Rocky, fortified region",
            "global_center" => [2000, -1000],
            "connections" => [
                ["direction" => "southeast", "global" => [2500, -1500], "to_map" => "Sítio Arqueológico Cahill", "to_direction" => "northwest", "to_global" => [2500, -1500]]
            ]
        ],
        [
            "name" => "Mina Lenfer",
            "type" => "Dark underground complex",
            "global_center" => [3000, -3000],
            "connections" => [
                ["direction" => "northeast", "global" => [3500, -2500], "to_map" => "Deserto Sigmund", "to_direction" => "southwest", "to_global" => [3500, -2500]]
            ]
        ],
        [
            "name" => "Floresta Ungor",
            "type" => "Large central forest",
            "global_center" => [3000, -3000],
            "connections" => [
                ["direction" => "north", "global" => [3000, -2500], "to_map" => "Sítio Arqueológico Cahill", "to_direction" => "south", "to_global" => [3000, -2500]],
                ["direction" => "east", "global" => [3500, -3000], "to_map" => "Floresta Cinzenta", "to_direction" => "west", "to_global" => [3500, -3000]],
                ["direction" => "southwest", "global" => [2500, -3500], "to_map" => "Floresta Zeelant", "to_direction" => "northeast", "to_global" => [2500, -3500]],
                ["direction" => "southeast", "global" => [3500, -3500], "to_map" => "Torre de Giovanni", "to_direction" => "northwest", "to_global" => [3500, -3500]]
            ]
        ],
        [
            "name" => "Floresta Cinzenta",
            "type" => "Dark, misty forest",
            "global_center" => [4000, -3000],
            "connections" => [
                ["direction" => "west", "global" => [3500, -3000], "to_map" => "Floresta Ungor", "to_direction" => "east", "to_global" => [3500, -3000]],
                ["direction" => "northeast", "global" => [4500, -2500], "to_map" => "Estrada Trifurcada Halperin", "to_direction" => "southwest", "to_global" => [3500, -3500]],
                ["direction" => "northeast", "global" => [4500, -2500], "to_map" => "Amarkand", "to_direction" => "southwest", "to_global" => [4500, -3500]],
                ["direction" => "southeast", "global" => [4500, -3500], "to_map" => "Torre de Giovanni", "to_direction" => "northeast", "to_global" => [4500, -3500]]
            ]
        ],
        [
            "name" => "Floresta Zeelant",
            "type" => "Wet, deep forest",
            "global_center" => [2000, -4000],
            "connections" => [
                ["direction" => "northeast", "global" => [2500, -3500], "to_map" => "Floresta Ungor", "to_direction" => "southwest", "to_global" => [2500, -3500]]
            ]
        ],
        [
            "name" => "Torre de Giovanni",
            "type" => "Mystic ruined tower",
            "global_center" => [4000, -4000],
            "connections" => [
                ["direction" => "northwest", "global" => [3500, -3500], "to_map" => "Floresta Ungor", "to_direction" => "southeast", "to_global" => [3500, -3500]],
                ["direction" => "northeast", "global" => [4500, -3500], "to_map" => "Floresta Cinzenta", "to_direction" => "southeast", "to_global" => [4500, -3500]]
            ]
        ]
    ]
];

// Capture JSON for debug
$jsonOutput = json_encode($mapData['maps'], JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_QUOT | JSON_HEX_APOS);

// Clear buffered output
ob_clean();
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>2D Map Visualization (Full WAMP)</title>
    <style>
        canvas {
            border: 1px solid black;
            margin: 20px;
        }
        body {
            font-family: Arial, sans-serif;
        }
        #debug, #error, #js-debug {
            margin: 20px;
            white-space: pre-wrap;
            font-family: monospace;
            font-size: 12px;
        }
        #error { color: red; }
        #js-debug { color: green; }
    </style>
</head>
<body>
    <h1>2D Map Visualization (Full WAMP)</h1>
    <p><strong>PHP Test:</strong> PHP is executing correctly if you see this message.</p>
    <p><strong>WAMP Info:</strong> PHP Version: <?php echo phpversion(); ?> | Timestamp: <?php echo date('Y-m-d H:i:s'); ?></p>
    <p><strong>Raw JSON:</strong> <a href="?raw_json=1">View raw JSON data</a></p>
    <p><strong>Note:</strong> Mina Lenfer is underground but shown at its XY position (3000, -3000) with a dashed outline.</p>
    <canvas id="mapCanvas" width="1200" height="1200"></canvas>
    <div id="debug">
        <h2>Debug: JSON Data (PHP)</h2>
        <pre><?php echo htmlspecialchars(json_encode($mapData, JSON_PRETTY_PRINT | JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_QUOT | JSON_HEX_APOS)); ?></pre>
    </div>
    <div id="js-debug">
        <h2>Debug: JavaScript JSON Output</h2>
        <pre>const maps = <?php echo htmlspecialchars($jsonOutput); ?>;</pre>
    </div>
    <div id="error"></div>

    <script>
        try {
            // Inject JSON data
            const maps = <?php echo $jsonOutput; ?>;
            console.log('Maps data:', maps);

            // Canvas setup
            const canvas = document.getElementById('mapCanvas');
            const ctx = canvas.getContext('2d');

            // Scaling and translation
            const scale = 0.08; // 1000 units -> 80 pixels
            const offsetX = 300;
            const offsetY = 650;

            // Color coding by map type
            const typeColors = {
                'City (human capital)': 'blue',
                'Trade town': 'cyan',
                'Desert with ruins': 'yellow',
                'Natural hub/transition area': 'purple',
                'Large central desert': 'orange',
                'Ruins and excavation site': 'brown',
                'Rocky, fortified region': 'gray',
                'Dark underground complex': 'black',
                'Large central forest': 'green',
                'Dark, misty forest': 'darkgreen',
                'Wet, deep forest': 'lime',
                'Mystic ruined tower': 'magenta'
            };

            // Draw maps
            maps.forEach(map => {
                const [x, y] = map.global_center;
                const canvasX = x * scale + offsetX;
                const canvasY = -y * scale + offsetY;
                const size = 800 * scale; // 800 units -> 64 pixels

                ctx.beginPath();
                if (map.name === 'Mina Lenfer') {
                    ctx.setLineDash([5, 5]); // Dashed for underground
                } else {
                    ctx.setLineDash([]);
                }
                ctx.rect(canvasX - size / 2, canvasY - size / 2, size, size);
                ctx.strokeStyle = typeColors[map.type] || 'blue';
                ctx.stroke();

                ctx.font = '10px Arial';
                ctx.fillStyle = 'black';
                ctx.textAlign = 'center';
                ctx.fillText(map.name, canvasX, canvasY - size / 2 - 5); // Offset above rectangle
            });

            // Draw connections
            maps.forEach(map => {
                map.connections.forEach(conn => {
                    const [x1, y1] = conn.global;
                    const [x2, y2] = conn.to_global;
                    const canvasX1 = x1 * scale + offsetX;
                    const canvasY1 = -y1 * scale + offsetY;
                    const canvasX2 = x2 * scale + offsetX;
                    const canvasY2 = -y2 * scale + offsetY;

                    ctx.beginPath();
                    ctx.moveTo(canvasX1, canvasY1);
                    ctx.lineTo(canvasX2, canvasY2);
                    ctx.strokeStyle = 'red';
                    ctx.stroke();

                    const angle = Math.atan2(canvasY2 - canvasY1, canvasX2 - canvasX1);
                    const arrowSize = 5;
                    ctx.beginPath();
                    ctx.moveTo(canvasX2, canvasY2);
                    ctx.lineTo(canvasX2 - arrowSize * Math.cos(angle - Math.PI / 6), canvasY2 - arrowSize * Math.sin(angle - Math.PI / 6));
                    ctx.moveTo(canvasX2, canvasY2);
                    ctx.lineTo(canvasX2 - arrowSize * Math.cos(angle + Math.PI / 6), canvasY2 - arrowSize * Math.sin(angle + Math.PI / 6));
                    ctx.stroke();

                    ctx.fillStyle = 'black';
                    ctx.font = '8px Arial';
                    ctx.fillText(`${conn.direction} → ${conn.to_direction}`, (canvasX1 + canvasX2) / 2, (canvasY1 + canvasY2) / 2);
                });
            });
        } catch (e) {
            document.getElementById('error').innerHTML = `JavaScript Error: ${e.message}\nLine: ${e.lineNumber}`;
            console.error('JavaScript Error:', e);
        }
    </script>
</body>
</html>
<?php
ob_end_flush();
?>
```