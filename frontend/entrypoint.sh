#!/bin/sh
# Cloud Run injects $PORT at runtime (usually 8080).
# Substitute it into the nginx config template before starting the server.
PORT="${PORT:-8080}"
envsubst '${PORT}' < /etc/nginx/conf.d/default.conf.template > /etc/nginx/conf.d/default.conf
exec nginx -g 'daemon off;'
