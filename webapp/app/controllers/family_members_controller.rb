class FamilyMembersController < ApplicationController
  def show
    @family_member = FamilyMember.find(params[:id])
  end
end
