Rails.application.routes.draw do
  root to: "family_members#index"

  get "/:id", to: "family_members#show", as: :family_member
end
