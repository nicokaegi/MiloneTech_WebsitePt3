from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from . import secrets


# the connection block so i can import to other files
conn = "mysql+pymysql://{0}:{1}@{2}/{3}".format(secrets.db_user, secrets.db_pass, secrets.db_host, secrets.db_name)
engine = create_engine(conn, echo=True)
Base = declarative_base()
Base.metadata.reflect(engine)
Session = sessionmaker(bind=engine)
session = Session()
