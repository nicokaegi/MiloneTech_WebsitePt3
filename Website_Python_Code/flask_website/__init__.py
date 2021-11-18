from flask import Flask, url_for
from flask_bcrypt import Bcrypt
import flask_website.dbAPI.app as db
from flask_login import LoginManager
from flask_admin import Admin
import sys

from flask_socketio import SocketIO
# from flask_website import routes


app = Flask(__name__)
app.config['SECRET_KEY'] = '5791628bb0b13ce0c676dfde280ba245'
bcrypt = Bcrypt(app)
login_manager = LoginManager(app)
admin = Admin(app, name='MiloneTech Admin Page', template_mode='bootstrap4')



socketio = SocketIO(app, logger=False)

from flask_website import routes
