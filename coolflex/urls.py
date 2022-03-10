from django.urls import path
from . import views

urlpatterns = [
  path("", views.home, name="home"),
  path("frontpage", views.frontpage, name="frontpage"),
  path("register", views.register_request, name="register"),
  path("login", views.login_request, name="login"),
  path("logout", views.logout, name="logout"),
]