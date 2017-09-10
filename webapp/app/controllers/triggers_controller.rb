class TriggersController < ApplicationController
  skip_before_filter :verify_authenticity_token

  def create
    if family_member = FamilyMember.find_by(device_id: params[:coreid])
      family_members.triggers.create(color: pparams[:color])

      # TODO: send triggers to
    end
  end

  private

  def pparams
    params.permit!
  end
end
