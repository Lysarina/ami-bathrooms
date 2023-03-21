# Publishes occupied status Occupied to bathroom 2
mosquitto_pub -d -t occupancy -u ella -P EllaPass3 -m "{\"occupancy\": 1, \"bathroomID\":2}" -h 165.22.31.23