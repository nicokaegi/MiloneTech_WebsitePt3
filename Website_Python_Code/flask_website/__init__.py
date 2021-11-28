from flask import Flask, url_for
from flask_bcrypt import Bcrypt
import flask_website.dbAPI.app as db
from flask_login import LoginManager
<<<<<<< HEAD
=======
from flask_admin import Admin
>>>>>>> 5719e991dc9a3214a63a84d20c2f02064841cf48
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
<<<<<<< HEAD


=======
>>>>>>> 5719e991dc9a3214a63a84d20c2f02064841cf48
