class AddDeviceIdToFamilyMembers < ActiveRecord::Migration[5.0]
  def change
    add_column :family_members, :device_id, :string
  end
end
