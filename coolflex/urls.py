from django.urls import path
from . import views
from django.contrib.auth.views import LogoutView
from django.conf import settings

urlpatterns = [
  path("", views.home, name="home"),
  path("info", views.info, name="info"),
  path("klasse", views.frontpage, name="frontpage"),
  path("create", views.CreateKlasse.as_view(), name="createklasse"),
  path("klasse/<pk>", views.klasse, name="klasse"),
  path("klasse/<pk>/update", views.UpdateKlasse.as_view(), name="updateklasse"),
  path("klasse/<pk>/delete", views.DeleteKlasse.as_view(), name="deleteklasse"),
  path("coouluser", views.coolUser, name="coouluser"),
  path("cooluser/create", views.CreateCoolUser.as_view(), name="createcooluser"),
  path("cooluser/<pk>/update", views.UpdateCoolUser.as_view(), name="updatecooluser"),
  path("cooluser/<pk>/delete", views.DeleteCoolUser.as_view(), name="deletecooluser"),
  path("register", views.register_request, name="register"),
  path("login", views.login_request, name="login"),
  path('logout/', LogoutView.as_view(next_page=settings.LOGOUT_REDIRECT_URL), name='logout'), #https://stackoverflow.com/questions/63445084/valueerror-at-logout-logout-didnt-return-an-httpresponse-object
]