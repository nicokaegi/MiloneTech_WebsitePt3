from flask import Flask
from flask_bcrypt import Bcrypt
import flask_website.dbAPI.app as db
from flask_login import LoginManager
from flask_admin.contrib.sqla import ModelView
from flask_admin import Admin

from flask_socketio import SocketIO
# from flask_website import routes


app = Flask(__name__)
app.config['SECRET_KEY'] = '5791628bb0b13ce0c676dfde280ba245'
bcrypt = Bcrypt(app)
login_manager = LoginManager(app)
admin = Admin(app, name='MiloneTech Admin Page', template_mode='bootstrap4')

class accounts(db.db.Base):
    __tablename__ = "accounts"
    extend_existing=True

class alerts(db.db.Base):
    __tablename__ = "alerts"
    extend_existing=True

class sensors(db.db.Base):
    __tablename__ = "sensors"
    extend_existing=True

admin.add_view(ModelView(accounts, db.db.session))
admin.add_view(ModelView(alerts, db.db.session))
admin.add_view(ModelView(sensors, db.db.session))


socketio = SocketIO(app, logger=False)



from flask_website import routes
