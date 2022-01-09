from flask_website import app, socketio
from flask_socketio import SocketIO

import os

# When you remove socketio, get rid of this line,
socket = SocketIO(app, cors_allowed_origins="*")

if __name__ == '__main__':
    print("getcwd():", os.getcwd())
    # and replace this
    socket.run(app, debug=True)
    # with this:
    # app.run()
