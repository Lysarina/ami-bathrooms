# Publishes occupied status Free to bathroom 2
mosquitto_pub -d -t occupancy -u ella -P EllaPass3 -m "{\"occupancy\": 0, \"bathroomID\":2}" -h 165.22.31.23